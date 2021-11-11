/***************************************************** 
** Name         : scanfile_threads.cpp
** Author       : cnj
** Version      : 1.0 
** Date         : 2021-11-11
** Description  : 多线程文件扫描
******************************************************/ 
#include "scanfile_threads.h"
//#include "crc32.hpp"
#include "cksum.hpp"

#include <iomanip>
#include <iterator>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

static void DisplayLocalIp();

CRCScanner::CRCScanner() : m_scan_count(0), m_wait(0), m_fsscan_done(false), m_fsoutput_done(false)
{
    m_num_threads = thread::hardware_concurrency() - 1;
	if (m_num_threads < 3){
		m_num_threads = 3;
	}
	cout << "Scan Thread Amount: " << m_num_threads << endl;
}

CRCScanner::CRCScanner(int _num_threads) : m_num_threads(_num_threads > 2 ? _num_threads : 3), m_wait(0), m_scan_count(0), m_fsscan_done(false), m_fsoutput_done(false)
{
	cout << "Scan Thread Amount: " << m_num_threads << endl;
}

void 
CRCScanner::ScanFile(fs::path& path)
{
    ifstream file(path.c_str(), ios::in | ios::binary);
    if (file.is_open())
    {
        //将扫描结果先记录到内存,再更新到日志会更快
        //进一步改进计划，使用mmap技术
		//char buf[1] = { 0 };
		//unsigned int count = 0;
		//while (!file.eof()) {
		//	file.read(buf, 1);
		//	count++;
		//}
		uint32_t crc = 0xFFFFFFFF;
		GZCRC32::calc_img_crc(path.c_str(), &crc);
		//速度太慢，麻省理工看来也是人啊
		//istream_iterator<uint8_t> head(file), tail;
		//crc = crc32<IEEE8023_CRC32_POLYNOMIAL>(crc, head, tail);
		/*
        ScanInfo* pInfo		= new ScanInfo();
        pInfo->file_dir		= path.parent_path();
        pInfo->file_name	= path.filename();
		pInfo->file_attr	= "F";
		pInfo->file_crc		= crc;
		pInfo->file_size	= ComputeFileSize(path);
		pInfo->update_time	= GetFileUpdateTime(path);
		unique_lock<mutex> lock(m_mtx_info);
        m_scan_infos.push(pInfo);
		m_scan_count++;
		lock.unlock();
		*/
		unique_lock<mutex> lock(m_mtx_info);
		time_t tt = GetFileUpdateTime(path);
		outfile
			<< "F" << " " 
			<< path.parent_path() << "/" << path.filename() << " "
			<< ComputeFileSize(path) << " "
			<< crc << " "
			<< asctime(localtime(&tt)) 
			<< endl;
		m_scan_count++;
		lock.unlock();
		if (m_scan_count % 100 == 0){
			std::cout << std::endl;
		} else {
			std::cout << ".";
		}		
    }
}

void
CRCScanner::ThreadOutput()
{       
    //将内存中的结果刷新到文件
    ofstream outfile;
	outfile.open("./filelist.txt");

	while (!m_fsoutput_done || !m_scan_infos.empty()) {
		unique_lock<mutex> lock(m_mtx_info);
        if (m_scan_infos.empty()) {
            lock.unlock();
            continue;
        }
		ScanInfo* pInfo = m_scan_infos.front();
		outfile
			<< pInfo->file_attr << " " 
			<< pInfo->file_dir << "/" << pInfo->file_name << " "
			<< pInfo->file_size << " "
			<< pInfo->file_crc << " "
			<< asctime(localtime(&pInfo->update_time)) 
			<< endl;
		m_scan_infos.pop();
		delete pInfo;
		lock.unlock();
    }
	outfile.close();
}

void 
CRCScanner::Output()
{
    ofstream outfile;
	outfile.open("./filelist.txt");

	int queue_size = m_scan_infos.size();
	for (int i = 0; i < queue_size; i++) {
		ScanInfo* pInfo = m_scan_infos.front();
		outfile
			<< pInfo->file_attr << " " 
			<< pInfo->file_dir << "/" << pInfo->file_name << " "
			<< pInfo->file_size << " "
			<< pInfo->file_crc << " "
			<< asctime(localtime(&pInfo->update_time)) 
			<< endl;
		m_scan_infos.pop();
		delete pInfo;
    }
	outfile.close();	
}

void 
CRCScanner::ThreadScan()
{
    while (!m_fsscan_done || !m_files.empty()) {
        unique_lock<mutex> lock(m_mtx_file);
        if (m_files.empty()) {
            lock.unlock();
            continue;
        }
        fs::path path = m_files.front();
        m_files.pop();
        lock.unlock();
        ScanFile(path);
    }
	m_wait++;
}

void 
CRCScanner::Scan(const string& path)
{    
	std::cout << "Scan Start " << std::endl;

	GZCRC32::init_crc_table();

	outfile.open("./filelist.txt");

    vector<thread*> thread_pool;
    for (unsigned int i = 0; i < m_num_threads; i++){
        thread_pool.push_back(new thread(&CRCScanner::ThreadScan, this));
	}

	//一边扫描一边写文件
	//不适用他的原因在下面
	//thread thread_outfile = thread(&CRCScanner::ThreadOutput, this);

    ScanDirectory(path);
    m_fsscan_done = true;

	//如果使用静态编译，并且线程结束太快，则运行下面的程序
	//则会发生段越界，gcc7的bug
	//替代使用 m_wait
    //for (int i = 0; i < m_num_threads; i++){
	//	if (thread_pool[i]->joinable()){
    //    	thread_pool[i]->join();
	//	}
    //}

	while (m_wait < m_num_threads){
		std::this_thread::sleep_for(std::chrono::microseconds(5));
	}

	outfile.close();	

	thread_pool.clear();

	std::cout << std::endl;
	std::cout << "Scan Complete " << std::endl;

	//一边扫描一边写文件
	//有可能还没开始写扫描就已经结束，而且输出队列恰好还没写入数据
	//则会导致没有任何输出程序就结束了
	//m_fsoutput_done = true;
	//thread_outfile.join();

	//这种方式需要提供足够的内存
	//Output();

	std::cout << "Scan File Amount: " << m_scan_count << std::endl;

	DisplayLocalIp();
}

void 
CRCScanner::ScanDirectory(fs::path directory)
{
    for (const auto & entry : fs::recursive_directory_iterator(directory)) 
	{
		if (fs::is_directory(entry.path())) {

			// 使用 recursive_directory_iterator
			// 就不在需要递归了
			// ScanDirectory(entry.path());

			// having bug 
			// directory been duplicate record
			/*
			ScanInfo* pInfo = new ScanInfo();
			pInfo->file_dir = directory;
			pInfo->file_attr = "D";
			unique_lock<mutex> lock(m_mtx_info);
			m_scan_infos.push(pInfo);
			lock.unlock();
			*/
		}
        else {
            if (!fs::is_regular_file(entry.path())) continue;
            unique_lock<mutex> lock(m_mtx_file);
            m_files.push(entry.path());
        }
    }
}

uintmax_t 
CRCScanner::ComputeFileSize(const fs::path& path)
{
	if (fs::exists(path) &&
		fs::is_regular_file(path))
	{
		auto err = std::error_code{};
		auto filesize = fs::file_size(path, err);
		if (filesize != static_cast<uintmax_t>(-1))
			return filesize;
	}

	return static_cast<uintmax_t>(-1);
}

time_t 
CRCScanner::GetFileUpdateTime(const fs::path& path)
{
	auto timeEntry = fs::last_write_time(path);
	time_t cftime = chrono::system_clock::to_time_t(timeEntry);
	return cftime;
}

void DisplayLocalIp()
{
	struct ifaddrs * ifAddrStruct = NULL;
	struct ifaddrs * ifa = NULL;
	void * tmpAddrPtr = NULL;

    printf("Local Ip\n");
    printf("=================\n");
	getifaddrs(&ifAddrStruct);

	for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
			// is a valid IP4 Address
			tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("'%s': %s\n", ifa->ifa_name, addressBuffer);
		}
		else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
		    // is a valid IP6 Address
			tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
			printf("'%s': %s\n", ifa->ifa_name, addressBuffer);
		}
	}
	if (ifAddrStruct != NULL)
		freeifaddrs(ifAddrStruct);
}