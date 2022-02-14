/***************************************************** 
** Name         : main.cpp
** Author       : cnj
** Version      : 1.0 
** Date         : 2021-11-11
** Description  : 增量文件扫描
******************************************************/ 
#include "crc_log.h"
#include "crc_config.h"
#include "scanfile_threads.h"
#include <chrono>
#include <algorithm>

int main(int argc, char* argv[])
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
	//当前全量文件缓存
	map<string, unsigned long long> FullListFileMap;
	//是否第一次运行
	bool IS_FIRST_RUN = false;

	//读取参数
	string	SCAN_DIRECTORY	= CRCConfig::Instance().getStr("scanDir","D:\\vs2015\\Grocery");
	string	CUR_LIST_FILE	= CRCConfig::Instance().getStr("curListFile", "");
	string	PRE_LIST_FILE	= CRCConfig::Instance().getStr("preListFile", "");
	size_t	THREAD_NUM		= CRCConfig::Instance().getInt("threadNum", 4);
	size_t	SEPRARTE_SIZE	= CRCConfig::Instance().getInt("seprarteSize", 128);
	size_t	DELAY_TIME		= CRCConfig::Instance().getInt("timer", 0);

	//文件列表的文件数按给定尺寸分割
	unsigned long long SEPRARTE_SIZE_BYTE = SEPRARTE_SIZE * 1024 * 1024 * 1024;
	//扫描文件前缀
	string	scanFilePre		= "MidasFullFileList-";
	//增量文件前缀
	string	deltaFilePre	= "MidasDeltaFileList-";

	//实例扫描
	CRCScanner Scanner(THREAD_NUM);

	//检查 CUR_FILE_LIST
	if (CUR_LIST_FILE.empty()) {	//找到当天的文件扫描列表文件
		auto t			= std::chrono::system_clock::now();
		auto tNow		= std::chrono::system_clock::to_time_t(t);
		tm*  now		= std::localtime(&tNow);
		char path[1024] = {};
		sprintf(path, ".\\%s%d%d%d.txt", scanFilePre.c_str(), now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
		CUR_LIST_FILE = path;
		CRCLog::Info("current file list: %s", CUR_LIST_FILE.c_str());
	}

	//找出所有的全量列表扫描文件。
	//如果列表为空，直接去扫描。
	//否则，找出当天的全量扫描文件，
	//以及距离当天的全量扫描文件最近的上一个全量扫描文件。
	//如果上一个全量扫描文件不存在，则将当前的全量赋值给
	//上一个全量扫描文件，并将当前的全量扫描文件名清除。
	//----------------------------------------------------
	bool	cur_list_file_exist = false;
	bool	pre_list_file_exist = false;
	int		MaxNo		= -1;
	string	filterName	= scanFilePre + "[0-9]+\\\.txt";
	vector<fs::path> FullListFiles;
	Scanner.ScanFileOfCurDir(FullListFiles, filterName);
	if (FullListFiles.empty()) {
		CRCLog::Info("Today Full Scan File<%s> is not Exist, NEED scan first", CUR_LIST_FILE.c_str());
	}
	else {
		//vector<fs::path>::iterator it = find(FullFileList.begin(), FullFileList.end(), CUR_LIST_FILE);
		//if (it == FullFileList.end()) {
		//	cur_list_file_exist = true;
		//}
		
		//找出距离当前列表文件时间最近的那个列表文件
		int index = 0;
		for (auto entry : FullListFiles) {
			
			index++;
			CRCLog::Info("entry: %s", entry.u8string().c_str());
			string ss  = entry.u8string();
			if (ss.compare(CUR_LIST_FILE) == 0) {
				cur_list_file_exist = true;
				continue;
			}
			CRCLog::Info("list file: %s", ss.c_str());
			string sno = ss.substr(ss.find_last_of("-")+1, ss.size());
			if (sno.empty()) { continue; }
			if (atoi(sno.c_str()) > MaxNo) {
				MaxNo = atoi(sno.c_str());
			}
			CRCLog::Info("max no: %d", MaxNo);
		}

		//如果存在距离当前列表文件时间最近的列表文件
		if (MaxNo > 0) {
			pre_list_file_exist = true;
			PRE_LIST_FILE = FullListFiles[--index].u8string();
		}
		else {
			PRE_LIST_FILE = CUR_LIST_FILE;
			CUR_LIST_FILE.clear();
		}

		CRCLog::Info("previous list file %s", PRE_LIST_FILE.c_str());

		//如果两个都不存在清除 FullListFiles
		if (!cur_list_file_exist && !pre_list_file_exist) {
			FullListFiles.clear();
			IS_FIRST_RUN = true;
		}
		else {
			//如果文件列表缓存不存在，则生成缓存
			if (FullListFileMap.empty()) {

				if (!fs::exists(PRE_LIST_FILE) || !fs::is_regular_file(PRE_LIST_FILE)) {
					CRCLog::Error("EXCEPTION %s is not exist", PRE_LIST_FILE.c_str());
					exit(0);
				}
				else {
					ifstream fin(PRE_LIST_FILE.c_str(), ios::in | ios::binary);
					if (fin.is_open()) {
						CRCLog_Info("Open PRE LIST FILE %s SUCCESS", PRE_LIST_FILE.c_str());
						int		skip = 5;
						string	line;
						while (getline(fin, line))
						{
							if (skip-->0){ 
								CRCLog::Info("%s", line.c_str());
								continue;
							}
							string name	= line.substr(0, line.find_first_of(" "));
							size_t size	= atoi(line.substr(line.find_last_of(" ") + 1, line.size()).c_str());
							CRCLog::Info("file: %s, size: %d", name.c_str(), size);
							FullListFileMap[name] = size;
						}
						CRCLog::Info("map size: %d", FullListFileMap.size());
					}
				}
				return 0;
			}
		}

	}
	//----------------------------------------------------


	//统计所有的增量文件的文件大小
	//如果大于128G则生成一个文件下载列表文件

	std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
	Scanner.Scan(SCAN_DIRECTORY);
	auto count = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count() * 0.000001;
	std::cout << "Elapsed Time: " << count << "(s)" << std::endl;

    return 0;
}