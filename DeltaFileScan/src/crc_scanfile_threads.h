/*****************************************************
** Name         : scanfile_threads.h
** Author       : chenningjiang
** Version      : 1.0
** Date         : 2022-02-15
** Description  : 多线程文件扫描
******************************************************/
#ifndef _CRC_SCANFILE_THREADS_H_
#define _CRC_SCANFILE_THREADS_H_

#include "crc_log.h"

#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <sstream>
#include <mutex>
#include <string>
#include <thread>
#include <queue>
#include <atomic>
#include <regex>
#include <condition_variable>

using namespace std;
namespace fs = std::experimental::filesystem;

struct ScanInfo    //扫描文件信息
{
	fs::path		file_dir;
	fs::path		file_name;
	string			file_full_name;
	unsigned int	file_size;
	uint32_t		file_crc;
	time_t			update_time;
};

class CRCScanner
{
private:
	unsigned            m_num_threads;
	unsigned            m_scan_count;
	atomic_int          m_wait;
	atomic_int          m_atomic{ 0 };
	bool                m_fsscan_done;
	bool                m_fsoutput_done;
	bool                m_need_crc;
	ofstream            m_outfile;
	mutex               m_mtx_file;
	mutex               m_mtx_info;
	queue<fs::path>     m_files;
	queue<ScanInfo*>    m_scan_infos;
	condition_variable	m_cond;
	bool				m_bTerminate = false;
	string				m_source_dir;
	unsigned long long  m_total_size;
	unsigned long       m_total_file;
	vector<thread*>		m_thread_pool;

	void ThreadScan(bool isQuick = false);
	void ScanFile(fs::path& file);
	void ScanFileQuick(fs::path& file);
	void ThreadOutput();
	bool Get(fs::path & path);
	bool WaitForAllDone(int millsecond=-1);
	void Stop();

public:
	CRCScanner();
	CRCScanner(int _num_threads);

	string		GetScanFile() { return m_source_dir; }

	void        SetNeedCRC(bool need);
	void        Scan(const string& path);
	void		ScanQuick(const string& path);
	void        ScanDirectory(fs::path directory);
	void		ScanFileOfCurDir(vector<fs::path> & outFiles, const string& _regex);
	uintmax_t   ComputeFileSize(const fs::path& path);
	time_t      GetFileUpdateTime(const fs::path& path);
};

#endif