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
//----------------------------------------------------
string	SCAN_DIRECTORY;								//扫描目录
string	TARGET_DIRECTORY;							//copy form SCAN_DIRECTORY to TARGET_DIRECTORY
string	CUR_LIST_FILE;								//当前全量扫描文件
string	PRE_LIST_FILE;								//上一次全量扫描文件
size_t	THREAD_NUM;									//扫描线程数
size_t	SEPRARTE_SIZE;								//增量文件分割尺寸
size_t	DELAY_TIME;									//下一次执行延时时间，从零点开始
string	TEST;										//用于测试
string  CYCLE;										//是否循环执行

bool	IS_FIRST_RUN	= false;					//是否第一次运行
string	SCANFILEPRE		= "XFullFileList";			//扫描文件前缀
string	DELTAFILEPRE	= "XDeltaFileList";			//增量文件前缀

unsigned long long	SEPRARTE_SIZE_BYTE;				//文件列表的文件数按给定尺寸分割

vector<fs::path>	FULLLISTFILES;
vector<fs::path>	DELTALISTFILES;
vector<fs::path>	MOVEFILES;

map<string, unsigned long long> FULLLISTFILEMAP;	//当前全量扫描缓存

size_t	SHOW_COUNT		= 0;
string	SHOW_DOT;

//----------------------------------------------------
void	AppendSuffix(string & fileName);
bool	CheckParam();
void	Init(int argc, char* argv[]);
string	QuickScan(CRCScanner& scanner);
void	ShowProcess();

//----------------------------------------------------
int main(int argc, char* argv[])
{
	Init(argc, argv);

	if (!CheckParam()) {
		return -1;
	}

	if (TEST.compare("true")) {
		DELAY_TIME = 1;							//1分钟
		SEPRARTE_SIZE_BYTE = 10 * 1024 * 1024;	//10M
	}

	queue<ScanInfo*> deltaQueue;
	ifstream fin;

	//老板催得紧，懒得封装优化了，直接goto
	CYCLE:

	//实例扫描
	CRCScanner Scanner(THREAD_NUM);

	//检查 CUR_FILE_LIST
	if (CUR_LIST_FILE.empty()) {	//找到当天的文件扫描列表文件
		time_t t = time(0);
		CUR_LIST_FILE = ".\\"+SCANFILEPRE;
		AppendSuffix(CUR_LIST_FILE);
		CRCLog::Info("current file list: %s", CUR_LIST_FILE.c_str());
	}

	//找出所有的全量列表扫描文件。
	//如果列表为空，直接去扫描。
	//否则，找出当天的全量扫描文件，
	//以及距离当天的全量扫描文件最近的上一个全量扫描文件。
	//如果上一个全量扫描文件不存在，则将当前的全量赋值给
	//上一个全量扫描文件，并将当前的全量扫描文件名清除。
	//----------------------------------------------------
	unsigned long long PreMaxNo = 0;
	bool	cur_list_file_exist = false;
	bool	pre_list_file_exist = false;
	string	filterName			= SCANFILEPRE + "-[0-9]+\\\s{1}[0-9]+\\\.txt";
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
							if (skip-->0){ 
								CRCLog::Info("%s", line.c_str());
								continue;
							}

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

	//扫描
	//----------------------------------------------------
	if (!cur_list_file_exist) {
		CUR_LIST_FILE = QuickScan(Scanner);
	}
	//----------------------------------------------------

	//计算增量
	//----------------------------------------------------
	//如果是第一次运行，当天的全量扫描文件就是增量文件
	if (IS_FIRST_RUN) {
		CRCLog::Info("IS FIRST RUN");
		if (fs::exists(CUR_LIST_FILE) && fs::is_regular_file(CUR_LIST_FILE)) {
			string deltaFileName = DELTAFILEPRE;
			deltaFileName
				.append(CUR_LIST_FILE.substr(CUR_LIST_FILE.find_first_of("-"), 16))
				.append(CUR_LIST_FILE.substr(CUR_LIST_FILE.find_first_of("-"), 16))
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

	if (CUR_LIST_FILE.compare(PRE_LIST_FILE) == 0) {
		goto END;
	}

	CRCLog::Info("compare current and previous scan files");
	fin.open(CUR_LIST_FILE.c_str(), ios::in | ios::binary);
	SHOW_COUNT = 0;
	SHOW_DOT.clear();
	if (fin.is_open()) {
		CRCLog_Info("Open CUR LIST FILE %s SUCCESS", CUR_LIST_FILE.c_str());
		int		skip		= 0;
		string	line;
		std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
		while (getline(fin, line))
		{
			if (skip-- > 0) {
				CRCLog::Info("%s", line.c_str());
				continue;
			}

			string name = line.substr(0, line.find_first_of("|"));
			size_t size = atoi(line.substr(line.find_last_of("|") + 1, line.size()).c_str());
			if (name.empty()) {
				continue;
			}
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
	CRCLog::Info("create TEMP delta list file");
	if (!deltaQueue.empty())
	{
		string	fileName;
		time_t	t		= time(0);
		char	tmp[32]	= { 0 };
		strftime(tmp, sizeof(tmp), "-%Y%m%d", localtime(&t));
		fileName
			.append(DELTAFILEPRE)
			.append(tmp)
			.append("-")
			.append(std::to_string(PreMaxNo).substr(0, 8))
			.append(" ")
			.append(std::to_string(PreMaxNo).substr(9, 6))
			.append(".txt");

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
		CRCLog::Info("DELETE file %s, because the file as same as previous file", CUR_LIST_FILE.c_str());
		remove(CUR_LIST_FILE.c_str());
	}
	//----------------------------------------------------

	DELTA:
	//搜索所用增量文件，生成最终增量文件
	//----------------------------------------------------
	CRCLog::Info("create FINAL delta list file");
	filterName = DELTAFILEPRE + "-[0-9]+\\\s{1}[0-9]+-[0-9]+\\\s{1}[0-9]+\\\.txt";
	Scanner.ScanFileOfCurDir(DELTALISTFILES, filterName);
	unsigned long long sizeOfFiles = 0;
	bool isEndCycle = false;
	SHOW_COUNT = 0;
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
					isEndCycle = true;
					break;
				}
			}
			MOVEFILES.push_back(entry);
			CRCLog::Info("READY copy delta file : %s", entry.c_str());
			if (isEndCycle) {
				fin.close();
				break;
			}
			fin.close();
		}
	}
	//----------------------------------------------------

	//移动增量列表文件中的文件至目标目录
	//----------------------------------------------------
	if (!MOVEFILES.empty())
	{
		string targetDir = TARGET_DIRECTORY + "\\";
		time_t t = time(0);
		char tmp[32] = { 0 };
		strftime(tmp, sizeof(tmp), "-%Y%m%d %H%M00", localtime(&t));
		targetDir
			.append(SCAN_DIRECTORY.substr(3))
			.append(tmp);
		SHOW_COUNT = 0;
		SHOW_DOT.clear();
		for (auto entry : MOVEFILES) 
		{
			fin.open(entry.c_str(), ios::in | ios::binary);
			if (fin.is_open()) {
				string line;
				while (getline(fin, line))
				{
					ShowProcess();

					string sPath = line.substr(0, line.find_first_of("|"));
					string dPath = targetDir + sPath.substr(2, sPath.find_last_of("\\")-2);
					string name  = sPath.substr(sPath.find_last_of("\\"));
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
							fs::copy_file(sPath, dPath + name);
						}
					}

				}
				fin.close();
			}
		}
	}
	//----------------------------------------------------

	//将已经使用过的增量文件移动到垃圾桶
	//----------------------------------------------------
	if (!MOVEFILES.empty())
	{
		for (auto entry : MOVEFILES)
		{
			if (fs::exists(entry) && fs::is_regular_file(entry)) {
				string dPath;
				dPath.append("trash\\").append(entry.u8string());
				fs::rename(entry, dPath);
			}
		}
	}
	//----------------------------------------------------

	END:
	CUR_LIST_FILE.clear();
	PRE_LIST_FILE.clear();
	MOVEFILES.clear();
	DELTALISTFILES.clear();
	FULLLISTFILES.clear();
	if (CYCLE.compare("yes") == 0) {
		while (--DELAY_TIME) {
			Sleep(1000 * 60);
		}
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

	if (DELAY_TIME > (7 * 24 * 60)) {
		CRCLog::Error("DELAYING to execution cannot exceed 7 days");
		return false;
	}

	if (SEPRARTE_SIZE < 1 || SEPRARTE_SIZE > 128) {
		CRCLog::Error("THE SEPRARTE size cannot be less than 1G or greater than 128G");
		return false;
	}

	return true;
}

void Init(int argc, char* argv[])
{
	//设置运行日志名称
	CRCLog		::Instance().setLogPath("log\\DeltaScan", "w", true);
	CRCConfig	::Instance().Init(argc, argv);

	//建立日志目录
	if (!fs::exists("log") || !fs::is_directory("log")) {
		fs::create_directories("log");
	}
	//建立垃圾桶
	if (!fs::exists("trash") || !fs::is_directory("trash")) {
		fs::create_directories("trash");
	}

	//读取参数
	SCAN_DIRECTORY		= CRCConfig::Instance().getStr("scanDir", "D:\\网络通讯引擎");
	TARGET_DIRECTORY	= CRCConfig::Instance().getStr("targetDir", "D:\\temp");
	CUR_LIST_FILE		= CRCConfig::Instance().getStr("curListFile", "");
	PRE_LIST_FILE		= CRCConfig::Instance().getStr("preListFile", "");
	THREAD_NUM			= CRCConfig::Instance().getInt("threadNum", 4);
	SEPRARTE_SIZE		= CRCConfig::Instance().getInt("seprarteSize", 25);
	DELAY_TIME			= CRCConfig::Instance().getInt("delay", 1);
	TEST				= CRCConfig::Instance().getStr("test", "yes");
	CYCLE				= CRCConfig::Instance().getStr("cycle", "no");
	
	//计算分割尺寸
	unsigned long long SEPRARTE_SIZE_BYTE = SEPRARTE_SIZE * 1024 * 1024 * 1024;

}

string QuickScan(CRCScanner& scanner)
{
	CRCLog::Info("START scan 5s", SCAN_DIRECTORY.c_str());
	std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
	scanner.ScanQuick(SCAN_DIRECTORY);
	auto count = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count() * 0.000001;
	CRCLog::Info("SCAN <%s> elapse time: %f(s)", SCAN_DIRECTORY.c_str(), count);
	return scanner.GetScanFile();
}

void ShowProcess()
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
