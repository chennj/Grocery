/***************************************************** 
** Name         : main.cpp
** Author       : chenningjiang
** Version      : 1.0 
** Date         : 2022-02-15
** Description  : 增量文件扫描
******************************************************/ 
#include "crc_log.h"
#include "crc_config.h"
#include "crc_scanfile_threads.h"
#include <chrono>
#include <algorithm>

#define	LYMD	8
#define LHMS	6
#define LYMDHMS 15

//----------------------------------------------------
static string	SCAN_DIRECTORY;									//扫描目录
static string	TARGET_DIRECTORY;								//copy form SCAN_DIRECTORY to TARGET_DIRECTORY
static string	CUR_LIST_FILE;									//当前全量扫描文件
static string	PRE_LIST_FILE;									//上一次全量扫描文件
static size_t	THREAD_NUM;										//扫描线程数
static size_t	SEPRARTE_SIZE;									//增量文件分割尺寸
static string	START_DATE;										//启动时间
static size_t	DELAY_TIME;										//下一次执行延时时间，从零点开始
static string	TEST;											//用于测试
static string	CYCLE;											//是否循环执行

static string	SCANFILEPRE		= "XFullFileList";				//扫描文件前缀
static string	DELTAFILEPRE	= "XDeltaFileList";				//增量文件前缀

static unsigned long long	SEPRARTE_SIZE_BYTE;					//文件列表的文件数按给定尺寸分割

vector<fs::path>	FULLLISTFILES;
vector<fs::path>	DELTALISTFILES;
vector<fs::path>	MOVEFILES;

static bool		IS_FIRST_RUN	= true;							//是否第一次运行
static map<string, unsigned long long> FULLLISTFILEMAP;			//当前全量扫描缓存

size_t	SHOW_COUNT		= 0;
string	SHOW_DOT;

//----------------------------------------------------
void	AppendSuffix(string & fileName);
bool	CheckParam();
void	Init(int argc, char* argv[]);
string	QuickScan(CRCScanner& scanner);
void	ShowProcessEx();										//用作显示进度，避免无聊
void	ShowProcess();
time_t	StringToDatetime(std::string str);

//----------------------------------------------------
int main(int argc, char* argv[])
{
	//建立日志目录
	if (!fs::exists("log") || !fs::is_directory("log")) {
		fs::create_directories("log");
	}

	//建立垃圾桶
	if (!fs::exists("trash") || !fs::is_directory("trash")) {
		fs::create_directories("trash");
	}

	CRCLog::Instance().setLogPath("log\\DeltaScan", "w");
	CRCConfig::Instance().Init(argc, argv);

	Init(argc, argv);

	//设置运行日志名称

	if (!CheckParam()) {
		return -1;
	}

	if (TEST.compare("true") == 0) {
		DELAY_TIME = 2;								//分钟
		SEPRARTE_SIZE_BYTE = 10 * 1024 * 1024;		//10M
	}

	unsigned long long	PreMaxNo;
	bool				cur_list_file_exist;
	bool				pre_list_file_exist;
	string				filterName;
	queue<ScanInfo*>	deltaQueue;
	ifstream			fin;
	unsigned long long	sizeOfFiles;
	bool				isMeetSizeRequire;			//是否满足尺寸大小要求

	//做一次性比较，不循环，此为手动方式
	if (!PRE_LIST_FILE.empty() && !CUR_LIST_FILE.empty()) {
		CRCLog::Info("此功能还没有完成，留着");
		return 0;
	}

	//老板催得紧，懒得封装优化了，直接goto
	CYCLE:

	//文件扫描实例
	CRCScanner Scanner(THREAD_NUM);

	//生成当前的扫描文件名
	CUR_LIST_FILE = ".\\" + SCANFILEPRE;
	AppendSuffix(CUR_LIST_FILE);
	CRCLog::Info("Current scan file name : %s", CUR_LIST_FILE.c_str());

	//找出所有的全量列表扫描文件。
	//如果列表为空，直接去扫描。
	//否则，找出当天的全量扫描文件，
	//以及距离当天的全量扫描文件最近的上一个全量扫描文件。
	//如果上一个全量扫描文件不存在，则将当前的全量赋值给
	//上一个全量扫描文件。
	//----------------------------------------------------
	PreMaxNo			= 0;
	cur_list_file_exist = false;
	pre_list_file_exist = false;
	filterName			= SCANFILEPRE + "-[0-9]+\\\s{1}[0-9]+\\\.txt";
	Scanner.ScanFileOfCurDir(FULLLISTFILES, filterName);
	if (FULLLISTFILES.empty()) {
		CRCLog::Info("Today Full Scan File<%s> is not Exist, NEED scan first", CUR_LIST_FILE.c_str());
		IS_FIRST_RUN = true;
	}
	else {
		//找出距离当前列表文件时间最近的那个列表文件
		int index = 0;
		for (int i = 0; i < FULLLISTFILES.size(); i++) 
		{			
			CRCLog::Info("full list file: %s", FULLLISTFILES[i].u8string().c_str());
			string ss  = FULLLISTFILES[i].u8string();
			if (ss.compare(CUR_LIST_FILE) == 0) {
				cur_list_file_exist = true;
				continue;
			}
			//提取文件名中的年月日时分秒，去掉中间的空格
			string symd = ss.substr(ss.find_last_of("-") + 1, LYMD);
			string shms = ss.substr(ss.find_last_of(" ") + 1, LHMS);
			string sno = symd + shms;

			if (sno.empty()) { continue; }
			if (stoull(sno.c_str()) > PreMaxNo) {
				PreMaxNo = stoull(sno.c_str());
				index = i;
			}
		}

		CRCLog::Info("PREVIOUS date no: %s", std::to_string(PreMaxNo).c_str());

		//如果存在距离当前列表文件时间最近的列表文件
		if (PreMaxNo > 0) {
			pre_list_file_exist = true;
			PRE_LIST_FILE = FULLLISTFILES[index].u8string();
		}
		else {
			PRE_LIST_FILE = CUR_LIST_FILE;
		}

		//如果两个都不存在清除 FullListFiles
		if (!cur_list_file_exist && !pre_list_file_exist) {
			FULLLISTFILES.clear();
			IS_FIRST_RUN = true;
			CRCLog::Warring("FULLLISTFILES is not empty, but CUR file OR PRE file is not exist");
		}
		else {
			//如果文件列表缓存不存在，则生成缓存
			IS_FIRST_RUN = false;
			CRCLog::Info("CREATE scan file cache");
			if (FULLLISTFILEMAP.empty()) {

				if (!fs::exists(PRE_LIST_FILE) || !fs::is_regular_file(PRE_LIST_FILE)) {
					CRCLog::Error("EXCEPTION %s is not exist", PRE_LIST_FILE.c_str());
					exit(0);
				}
				else {
					SHOW_COUNT = 0;
					SHOW_DOT.clear();
					fin.open(PRE_LIST_FILE.c_str(), ios::in | ios::binary);
					if (fin.is_open()) {
						CRCLog_Info("Open PRE LIST FILE %s SUCCESS", PRE_LIST_FILE.c_str());
						int		skip		= 0;
						string	line;
						while (getline(fin, line))
						{
							//if (skip-->0){ 
							//	CRCLog::Info("%s", line.c_str());
							//	continue;
							//}

							string name	= line.substr(0, line.find_first_of("|"));
							size_t size	= atoi(line.substr(line.find_last_of("|") + 1, line.size()).c_str());
							//CRCLog::Info("file: %s, size: %d", name.c_str(), size);
							if (name.empty()) {
								continue;
							}

							ShowProcess();

							//if (FullListFileMap.find(name) != FullListFileMap.end()) {
							//	CRCLog::Info("duplicate key name: %s", name.c_str());
							//}
							FULLLISTFILEMAP[name] = size;
						}
						CRCLog::Info("map size: %d, count: %d", FULLLISTFILEMAP.size(), SHOW_COUNT);
						fin.close();
					}
				}
			}
		}

	}
	//----------------------------------------------------

	//如果当前文件不存在，扫描
	//----------------------------------------------------
	if (!cur_list_file_exist) {
		CUR_LIST_FILE = QuickScan(Scanner);
	}
	//----------------------------------------------------

	//计算增量
	//----------------------------------------------------
	//如果扫描目录是空目录，会出问题，所以睡一会儿
	Sleep(1);
	//如果是第一次运行，当天的全量扫描文件就是增量文件
	if (IS_FIRST_RUN) {
		CRCLog::Info("IS FIRST RUN");
		if (fs::exists(CUR_LIST_FILE) && fs::is_regular_file(CUR_LIST_FILE)) {
			string deltaFileName = DELTAFILEPRE;
			deltaFileName
				.append(CUR_LIST_FILE.substr(CUR_LIST_FILE.find_first_of("-"), LYMDHMS + 1))
				.append(CUR_LIST_FILE.substr(CUR_LIST_FILE.find_first_of("-"), LYMDHMS + 1))
				.append(".txt");
			if (fs::exists(deltaFileName) && fs::is_regular_file(deltaFileName)) {
				CRCLog::Error("DELTA FILE [%s] IS EXIST, CANN'T COPY", deltaFileName.c_str());
				exit(0);
			}
			fs::copy_file(CUR_LIST_FILE, deltaFileName);
		}
		else {
			CRCLog::Error("IS FIRST RUN, but current file [] is not exist!", CUR_LIST_FILE);
			exit(0);
		}
		goto DELTA;
	}

	//如果当前扫描文件与上一次扫描文件同名，则跳过，进入下一次循环
	if (CUR_LIST_FILE.compare(PRE_LIST_FILE) == 0) {
		goto END;
	}
	//----------------------------------------------------

	//遍历当前扫描文件，与缓存中的数据对比
	//如果缓存中没有，将其放入 FULLLISTFILEMAP，
	//并记录进增量缓存 deltaQueue 中
	//----------------------------------------------------
	CRCLog::Info("Compare Current and Previous scan files");
	fin.open(CUR_LIST_FILE.c_str(), ios::in | ios::binary);
	SHOW_COUNT = 0;
	SHOW_DOT.clear();
	if (fin.is_open()) {
		CRCLog_Info("Open CUR LIST FILE %s SUCCESS", CUR_LIST_FILE.c_str());
		int		skip		= 0;
		string	line;
		std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
		//遍历当前扫描文件
		while (getline(fin, line))
		{
			//if (skip-- > 0) {
			//	CRCLog::Info("%s", line.c_str());
			//	continue;
			//}

			string name = line.substr(0, line.find_first_of("|"));
			size_t size = atoi(line.substr(line.find_last_of("|") + 1, line.size()).c_str());
			if (name.empty()) {
				continue;
			}
			//检查当前扫描文件中的数据是否已经在缓存中
			if (FULLLISTFILEMAP.find(name) == FULLLISTFILEMAP.end()) {
				//CRCLog::Info("delta file name: %s, size: %d", name.c_str(), size);
				ScanInfo* pInfo = new ScanInfo;
				pInfo->file_full_name = name;
				pInfo->file_size = size;
				deltaQueue.push(pInfo);
				FULLLISTFILEMAP[name] = size;
			}

			ShowProcess();
		}
		CRCLog::Info("FULLLISTFILEMAP size: %d", FULLLISTFILEMAP.size());
		fin.close();
		auto count = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count() * 0.000001;
		CRCLog::Info("Delta compare file <%s> elapsed time: %f(s)", CUR_LIST_FILE.c_str(), count);
	}
	else {
		CRCLog::Error("EXCEPTION CUR FILE <%s> IS NOT EXISTS", CUR_LIST_FILE.c_str());
	}
	//----------------------------------------------------

	//生成增量文件
	//----------------------------------------------------
	CRCLog::Info("Ready to create TEMP delta list file, SIZE: %d", deltaQueue.size());
	if (!deltaQueue.empty())
	{
		string	fileName;
		//组装增量文件名
		fileName
			.append(DELTAFILEPRE)
			.append("-")
			.append(CUR_LIST_FILE.substr(CUR_LIST_FILE.find_first_of("-") + 1, LYMDHMS))
			.append("-")
			.append(std::to_string(PreMaxNo).substr(0, LYMD))
			.append(" ")
			.append(std::to_string(PreMaxNo).substr(9, LHMS))
			.append(".txt");

		//缓存中的数据记录进增量文件
		ofstream deltaFile;
		deltaFile.open(fileName.c_str());
		while(!deltaQueue.empty()){
			ScanInfo* entry = deltaQueue.front();
			deltaFile
				<< entry->file_full_name << "|"
				<< entry->file_size
				<< endl;
			deltaQueue.pop();
			delete entry;
		}
		deltaFile.close();
	}
	else {
		CRCLog::Info("DELETE Full scan file %s, BECAUSE the file as same as Previous file", CUR_LIST_FILE.c_str());
		remove(CUR_LIST_FILE.c_str());
	}
	//----------------------------------------------------

	DELTA:
	//搜索所用增量文件，将符合条件的文件放入缓存
	//----------------------------------------------------
	filterName = DELTAFILEPRE + "-[0-9]+\\\s{1}[0-9]+-[0-9]+\\\s{1}[0-9]+\\\.txt";
	Scanner.ScanFileOfCurDir(DELTALISTFILES, filterName);
	CRCLog::Info("Ready create FINAL delta list file cache, SIZE: %d", DELTALISTFILES.size());
	sizeOfFiles			= 0;
	isMeetSizeRequire	= false;
	SHOW_COUNT			= 0;
	SHOW_DOT.clear();
	for (auto entry : DELTALISTFILES) 
	{
		fin.open(entry.c_str(), ios::in | ios::binary);
		if (fin.is_open()) {
			string	line;
			while (getline(fin, line))
			{
				ShowProcess();

				size_t size = atoi(line.substr(line.find_last_of("|") + 1, line.size()).c_str());
				sizeOfFiles += size;
				if (sizeOfFiles >= SEPRARTE_SIZE_BYTE) {
					isMeetSizeRequire = true;
					break;
				}
			}
			CRCLog::Info("READY to use delta file : %s", entry.u8string().c_str());
			if (isMeetSizeRequire) {
				MOVEFILES.push_back(entry);
				fin.close();
				break;
			}
			MOVEFILES.push_back(entry);
			fin.close();
		}
	}
	//检查缓存里面的文件是否满足传输数据的大小
	//如果不满足，则清空
	if (!isMeetSizeRequire) {
		MOVEFILES.clear();
	}
	//----------------------------------------------------

	//移动增量列表文件中的文件至目标目录
	//----------------------------------------------------
	CRCLog::Info("Ready to detect whether there are qualified Delta files, SIZE: %d", MOVEFILES.size());
	if (!MOVEFILES.empty())
	{
		CRCLog::Info("START TRANSFERRING files to destination ADDRESS: %s", TARGET_DIRECTORY.c_str());
		SHOW_COUNT = 0;
		SHOW_DOT.clear();

		//组装目标目录名
		string	targetDir	= TARGET_DIRECTORY + "\\";
		time_t	t			= time(0);
		char	tmp[32]		= { 0 };
		strftime(tmp, sizeof(tmp), "-%Y%m%d %H%M00", localtime(&t));
		targetDir
			.append(SCAN_DIRECTORY.substr(3))
			.append(tmp);
		//拷贝原扫描路径的增量文件到目的目录中
		for (auto entry : MOVEFILES) 
		{
			fin.open(entry.c_str(), ios::in | ios::binary);
			if (fin.is_open()) {
				string line;
				while (getline(fin, line))
				{
					//显示进度，避免无聊
					ShowProcess();

					string sPath = line.substr(0, line.find_first_of("|"));
					string dPath = targetDir + sPath.substr(2, sPath.find_last_of("\\")-2);
					string name  = sPath.substr(sPath.find_last_of("\\"));

					//判断目的目录是否已经建立，没有则建立
					//否则直接拷贝
					if (fs::exists(dPath) && fs::is_directory(dPath)){
						auto ret = fs::copy_file(sPath, dPath + name);
						if (!ret) {
							CRCLog::Error("COPY FILE %s FAILED", sPath.c_str());
						}
					}
					else {
						auto ret = fs::create_directories(dPath);
						if (!ret) {
							CRCLog::Error("CREATE DIRECOTRYS %s FAILED", dPath.c_str());
						}
						else {
							auto ret = fs::copy_file(sPath, dPath + name);
							if (!ret) {
								CRCLog::Error("COPY FILE %s FAILED", sPath.c_str());
							}
						}
					}

				}
				fin.close();
			}
		}//!for (auto entry : MOVEFILES) 

		//传送烧录和打印文件
		string printFile	= "printfile.html";
		string burnFile		= "MIDASBURN.txt";

		if (fs::exists(printFile) && fs::is_regular_file(printFile)) {
			string dPath = targetDir + "\\" + printFile;
			auto ret = fs::copy_file(printFile, dPath);
			if (!ret) {
				CRCLog::Error("COPY FILE %s FAILED", printFile.c_str());
			}
		}
		else {
			CRCLog::Error("PRINT FILE %s IS NOT EXIST", printFile.c_str());
		}

		if (fs::exists(burnFile) && fs::is_regular_file(burnFile)) {
			string dPath = targetDir + "\\" + burnFile;
			auto ret = fs::copy_file(burnFile, dPath);
			if (!ret) {
				CRCLog::Error("COPY FILE %s FAILED", burnFile.c_str());
			}
		}
		else {
			CRCLog::Error("BURN FILE %s IS NOT EXIST", burnFile.c_str());
		}

	}
	//----------------------------------------------------

	//将已经使用过的增量文件移动到垃圾桶
	//----------------------------------------------------
	CRCLog::Info("Ready to PUT the used delta files into the Trash can, SIZE: %d", MOVEFILES.size());
	if (!MOVEFILES.empty())
	{
		for (auto entry : MOVEFILES)
		{
			if (fs::exists(entry) && fs::is_regular_file(entry)) {
				string dPath;
				dPath.append("trash\\").append(entry.u8string());
				CRCLog::Info("--Put %s Into Trash can", entry.u8string().c_str());
				fs::rename(entry, dPath);
			}
		}
	}
	//----------------------------------------------------

	END:
	//PRE_LIST_FILE = CUR_LIST_FILE;
	IS_FIRST_RUN = false;
	CUR_LIST_FILE.clear();
	PRE_LIST_FILE.clear();
	MOVEFILES.clear();
	DELTALISTFILES.clear();
	FULLLISTFILES.clear();
	if (CYCLE.compare("yes") == 0) {
		size_t odt = DELAY_TIME;
		while (DELAY_TIME--) {
			Sleep(1000 * 60);
		}
		DELAY_TIME = odt;
		goto CYCLE;
	}

    return 0;
}

void AppendSuffix(string & fileName)
{
	time_t t = time(0);
	char tmp[32] = { 0 };
	strftime(tmp, sizeof(tmp), "-%Y%m%d %H%M00", localtime(&t));
	fileName.append(tmp);
	fileName.append(".txt");
}

bool CheckParam()
{
	if (SCAN_DIRECTORY.empty() || !fs::exists(SCAN_DIRECTORY) || !fs::is_directory(SCAN_DIRECTORY) )
	{
		CRCLog::Error("SCAN directory does not exist OR no access");
		return false;
	}

	if (TARGET_DIRECTORY.empty() || !fs::exists(TARGET_DIRECTORY) || !fs::is_directory(TARGET_DIRECTORY))
	{
		CRCLog::Error("TARGET directory does not exist OR no access");
		return false;
	}

	if (DELAY_TIME > (7 * 24 * 60) || DELAY_TIME < 2) {
		CRCLog::Error("DELAYING to execution cannot exceed 7 days OR less then 2 min");
		return false;
	}

	if (SEPRARTE_SIZE < 1 || SEPRARTE_SIZE > 128) {
		CRCLog::Error("THE SEPRARTE size cannot be less than 1G or greater than 128G");
		return false;
	}

	if (!START_DATE.empty()) {
		string sRegex = "20[23]{1}[2-9]{1}-[0-1]{1}-[0-2]{2}\\\s{1}[0-9]";
		//...
	}

	return true;
}

void Init(int argc, char* argv[])
{
	//读取参数
	SCAN_DIRECTORY		= CRCConfig::Instance().getStr("scanDir", "D:\\书籍");
	TARGET_DIRECTORY	= CRCConfig::Instance().getStr("targetDir", "D:\\temp");
	CUR_LIST_FILE		= CRCConfig::Instance().getStr("curListFile", "");
	PRE_LIST_FILE		= CRCConfig::Instance().getStr("preListFile", "");
	THREAD_NUM			= CRCConfig::Instance().getInt("threadNum", 4);
	SEPRARTE_SIZE		= CRCConfig::Instance().getInt("seprarteSize", 25);
	START_DATE			= CRCConfig::Instance().getStr("scanDir", "");
	DELAY_TIME			= CRCConfig::Instance().getInt("delay", 5);
	TEST				= CRCConfig::Instance().getStr("test", "no");
	CYCLE				= CRCConfig::Instance().getStr("cycle", "yes");
	
	//计算分割尺寸
	SEPRARTE_SIZE_BYTE	= SEPRARTE_SIZE * 1024 * 1024 * 1024;

}

string QuickScan(CRCScanner& scanner)
{
	CRCLog::Info("START scan %s", SCAN_DIRECTORY.c_str());
	std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
	scanner.ScanQuick(SCAN_DIRECTORY);
	//scanner.Scan(SCAN_DIRECTORY);
	auto count = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count() * 0.000001;
	CRCLog::Info("SCAN <%s> elapse time: %f(s)", SCAN_DIRECTORY.c_str(), count);
	return scanner.GetScanFile();
}

void ShowProcessEx()
{
	SHOW_COUNT++;
	if (SHOW_COUNT % 100 == 0) {
		CRCLog::Debug("%s%d", SHOW_DOT.c_str(), SHOW_COUNT);
		SHOW_DOT.clear();
	}
	else {
		SHOW_DOT.append(".");
	}
}

void ShowProcess() {

	SHOW_COUNT++;
	if (SHOW_COUNT % 100 == 0) {
		cout << SHOW_DOT.c_str() << SHOW_COUNT << endl;
		SHOW_DOT.clear();
	}
	else {
		SHOW_DOT.append(".");
	}
}

time_t StringToDatetime(std::string str)
{
	char *cha = (char*)str.data();             // 将string转换成char*。
	tm tm_;                                    // 定义tm结构体。
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
	tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
	tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
	tm_.tm_mday = day;                         // 日。
	tm_.tm_hour = hour;                        // 时。
	tm_.tm_min = minute;                       // 分。
	tm_.tm_sec = second;                       // 秒。
	tm_.tm_isdst = 0;                          // 非夏令时。
	time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式。
	return t_;                                 // 返回值。
}