/***************************************************** 
** Name         : scanfile_threads.h 
** Author       : cnj
** Version      : 1.0 
** Date         : 2021-11-11
** Description  : 多线程文件扫描
******************************************************/ 
#ifndef _SCANFILE_THREADS_H_
#define _SCANFILE_THREADS_H_

#include <iostream>
#include <experimental/filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <queue>
#include <atomic>

using namespace std;
namespace fs = std::experimental::filesystem;

struct ScanInfo    //扫描文件信息
{
	fs::path		file_dir;
	fs::path		file_name;
	string			file_attr;
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
    bool                m_fsscan_done;
    bool                m_fsoutput_done;
    ofstream            outfile;
    mutex               m_mtx_file;
    mutex               m_mtx_info;
    queue<fs::path>     m_files;
    queue<ScanInfo*>    m_scan_infos;

    void ThreadScan();
    void ScanFile(fs::path& file);
    void ThreadOutput();
    void Output();

public:
    CRCScanner();
    CRCScanner(int _num_threads);

    void        Scan(const string& path);
    void        ScanDirectory(fs::path directory);
	uintmax_t   ComputeFileSize(const fs::path& path);
	time_t      GetFileUpdateTime(const fs::path& path);
};

#endif