#include "scanfile_threads.h"

#include <iomanip>
#include <iterator>
#include <string.h> 

static int InsertByLineNumber(string filePath, string insertContent, int insertLine, int gap = 1);

CRCScanner::CRCScanner() : m_scan_count(0), m_wait(0), m_fsscan_done(false), m_fsoutput_done(false), m_need_crc(true)
{
	m_num_threads = thread::hardware_concurrency() - 1;
	if (m_num_threads < 3) {
		m_num_threads = 3;
	}
	cout << "Scan Thread Amount: " << m_num_threads << endl;
}

CRCScanner::CRCScanner(int _num_threads)
	:
	m_num_threads(_num_threads > 2 ? _num_threads : 3),
	m_wait(0), m_scan_count(0), m_fsscan_done(false), m_fsoutput_done(false), m_need_crc(true)
{
	m_total_size = 0;
	m_total_file = 0;
	cout << "Scan Thread Amount: " << m_num_threads << endl;
}

void
CRCScanner::SetNeedCRC(bool need)
{
	m_need_crc = need;
	if (!m_need_crc) {
		cout << "No Calculate CRC32" << endl;
	}
}

void
CRCScanner::ScanFile(fs::path& path)
{
	ifstream file(path.c_str(), ios::in | ios::binary);
	if (file.is_open())
	{
		unique_lock<mutex> lock(m_mtx_info);
		time_t tt = GetFileUpdateTime(path);
		char tmp[32] = { 0 };
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&tt));
		tmp[sizeof(tmp) - 2] = '\0';	//去掉回车\r
		unsigned long size = ComputeFileSize(path);
		m_total_size += size;
		m_total_file++;
		m_outfile
			<< path.parent_path() << "\\" << path.filename() << " "
			<< string(tmp) << " "
			<< size
			<< endl;
		m_scan_count++;
		lock.unlock();
		if (m_scan_count % 100 == 0) {
			std::cout << m_scan_count << std::endl;
		}
		else {
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
	outfile.open("filelist.txt");

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

	time_t t = time(0);
	string filename = "MidasFullFileList";
	char tmp[32] = { 0 };
	strftime(tmp, sizeof(tmp), "-%Y%m%d", localtime(&t));
	filename.append(tmp);
	filename.append(".txt");

	m_outfile.open(filename.c_str());

	vector<thread*> thread_pool;
	for (unsigned int i = 0; i < m_num_threads; i++) {
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

	while (m_wait < m_num_threads) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	m_outfile.close();

	thread_pool.clear();

	std::cout << std::endl;
	std::cout << "Scan Complete " << std::endl;
	std::cout << "Scan File Amount: " << m_scan_count << std::endl;
	std::cout << "Write File Label, Please Waiting... " << endl;

	string filestringlabel;
	filestringlabel
		.append(filename.substr(0, filename.size() - 4))
		.append("\nSourceDir:").append(path);
	{
		stringstream ss;
		ss << m_total_file;
		filestringlabel
			.append("\nTotalFilesCount:").append(ss.str());
	}
	{
		stringstream ss;
		ss << m_total_size;
		filestringlabel
			.append("\nTotalFileSize:").append(ss.str())
			.append("\n####");
	}

	InsertByLineNumber(filename, filestringlabel.c_str(), 1);

	std::cout << "Write File Complete. " << endl;
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
			// 不写目录了
			//unique_lock<mutex> lock(m_mtx_info);
			//time_t tt = GetFileUpdateTime(entry.path());
			//char tmp[32] = { 0 };
			//strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&tt));
			//outfile
			//	<< entry.path() << " "
			//	<< tmp << " "
			//	<< "D"
			//	<< endl;
			//m_scan_count++;
			//lock.unlock();
			//if (m_scan_count % 100 == 0){
			//	std::cout << m_scan_count << std::endl;
			//} else {
			//	std::cout << ".";
			//}		

		}
		else {
			if (!fs::is_regular_file(entry.path())) continue;
			unique_lock<mutex> lock(m_mtx_file);
			m_files.push(entry.path());
		}
	}
}

void
CRCScanner::ScanFileOfCurDir(vector<fs::path> & outFiles, const string& _regex)
{
	for (const auto & entry : fs::directory_iterator("."))
	{
		if (fs::is_directory(entry.path())) {
			continue;
		}
		else {
			if (!fs::is_regular_file(entry.path())) { continue; }
			if (_regex.empty()) {
				outFiles.push_back(entry.path());
			}
			else {
				auto temp = entry.path().filename();
				if (std::regex_match(temp.string(), regex(_regex))) {
					outFiles.push_back(entry.path());
				}
			}
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

int
InsertByLineNumber(string filePath, string insertContent, int insertLine, int gap)
{
	if (filePath == "" || insertContent == "" || gap <= 0 || insertLine < 0)
	{
		return -1;
	}
	ifstream is(filePath);
	if (!is.is_open())
	{
		cerr << "文件打开失败" << endl;
		return -1;
	}
	string tmp;
	string buffer;
	int count = 0;
	while (getline(is, tmp, '\n'))
	{
		count++;
		if (count == insertLine)
		{
			buffer += insertContent;
			buffer += "\n";
		}
		else if (count > insertLine && count < insertLine + gap)
		{
			continue;
		}
		else
		{
			buffer += tmp;
			buffer += "\n";
		}
	}
	is.close();
	ofstream os(filePath);
	os << buffer;
	os.close();
	return 0;
}