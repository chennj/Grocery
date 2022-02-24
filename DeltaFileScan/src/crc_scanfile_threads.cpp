#include "crc_scanfile_threads.h"

#include <iomanip>
#include <iterator>
#include <string.h> 

static int InsertByLineNumber(string filePath, string insertContent, int insertLine, int gap = 1);
static string show_dot;
static const uintmax_t UILL = static_cast<uintmax_t>(-1);

CRCScanner::CRCScanner() : m_scan_count(0), m_wait(0), m_fsscan_done(false), m_fsoutput_done(false), m_need_crc(true)
{
	m_num_threads = thread::hardware_concurrency() - 1;
	if (m_num_threads < 3) {
		m_num_threads = 3;
	}
	CRCLog::Info("Scan Thread Amount: %d", m_num_threads);
}

CRCScanner::CRCScanner(int _num_threads)
	:
	m_num_threads(_num_threads > 2 ? _num_threads : 3),
	m_wait(0), m_scan_count(0), m_fsscan_done(false), m_fsoutput_done(false), m_need_crc(true)
{
	m_total_size = 0;
	m_total_file = 0;
	CRCLog::Info("Scan Thread Amount: %d", m_num_threads);
}

void
CRCScanner::SetNeedCRC(bool need)
{
	m_need_crc = need;
	if (!m_need_crc) {
		CRCLog::Info("No Calculate CRC32");
	}
}

void
CRCScanner::ScanFile(fs::path& path)
{
	ifstream file(path.c_str(), ios::in | ios::binary);
	if (file.is_open())
	{
		time_t tt = GetFileUpdateTime(path);
		char tmp[32] = { 0 };
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&tt));
		tmp[sizeof(tmp) - 2]	= '\0';	//去掉回车\r
		unsigned long size		= ComputeFileSize(path);
		//fs::path &parentPath	= path.parent_path();
		//fs::path &fileName	= path.filename();

		unique_lock<mutex> lock(m_mtx_info);
		m_total_size += size;
		m_total_file++;
		m_outfile
			<< path.parent_path() << "\\" << path.filename() << "|"
			<< string(tmp) << "|"
			<< size
			<< endl;
		m_scan_count++;
		if (m_scan_count % 100 == 0) {
			//CRCLog::Debug("%s%d", show_dot.c_str(), m_scan_count);
			cout << show_dot.c_str() << m_scan_count << endl;
			show_dot.clear();
		}
		else {
			show_dot.append(".");
		}
		lock.unlock();
	}
}

void CRCScanner::ScanFileQuick(fs::path & path)
{
	if (fs::exists(path) && fs::is_regular_file(path))
	{
		ScanInfo* pInfo		= new ScanInfo;
		pInfo->update_time	= GetFileUpdateTime(path);
		uintmax_t uill		= ComputeFileSize(path);
		if (uill == UILL) {
			CRCLog::Error("FILE (%s) SIZE is -1, skip it.", path.c_str());
			return;
		}
		pInfo->file_size	= uill;
		pInfo->file_dir		= path.parent_path();
		pInfo->file_name	= path.filename();

		unique_lock<mutex> lock(m_mtx_info);
		m_total_size += pInfo->file_size;
		m_total_file++;
		m_scan_infos.push(pInfo);
		m_scan_count++;
		if (m_scan_count % 100 == 0) {
			//CRCLog::Debug("%s%d", show_dot.c_str(), m_scan_count);
			cout << show_dot.c_str() << m_scan_count << endl;
			show_dot.clear();
		}
		else {
			show_dot.append(".");
		}
		lock.unlock();
	}

}

void
CRCScanner::ThreadOutput()
{
	//将内存中的结果刷新到文件
	time_t t = time(0);
	m_source_dir = "XFullFileList";
	char tmp[32] = { 0 };
	strftime(tmp, sizeof(tmp), "-%Y%m%d %H%M00", localtime(&t));
	m_source_dir.append(tmp);
	m_source_dir.append(".txt");

	m_outfile.open(m_source_dir.c_str());

	while (m_wait < m_num_threads || !m_scan_infos.empty()) {
		unique_lock<mutex> lock(m_mtx_info);
		if (m_scan_infos.empty()) {
			lock.unlock();
			continue;
		}
		ScanInfo* pInfo = m_scan_infos.front();
		m_scan_infos.pop();
		lock.unlock();

		char tmp[32] = { 0 };
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&pInfo->update_time));
		tmp[sizeof(tmp) - 2] = '\0';	//去掉回车\r
		m_outfile
			<< pInfo->file_dir << "\\" << pInfo->file_name << "|"
			<< string(tmp) << "|"
			<< pInfo->file_size
			<< endl;
		
		delete pInfo;		
	}

	m_outfile.close();
}

bool CRCScanner::Get(fs::path & path)
{
	std::unique_lock<std::mutex> lock(m_mtx_file);

	if (m_files.empty()) {
		m_cond.wait(lock, [this] {return m_bTerminate || !m_files.empty(); });
	}

	if (m_bTerminate) return false;

	if (!m_files.empty())
	{
		path = move(m_files.front());
		m_files.pop();
		return true;
	}

	return false;
}

bool CRCScanner::WaitForAllDone(int millsecond)
{
	std::unique_lock<std::mutex> lock(m_mtx_file);

	if (m_files.empty()) {
		return true;
	}

	if (millsecond < 0) {
		m_cond.wait(lock, [this] {return m_files.empty(); });
		return true;
	}
	else {
		return m_cond.wait_for(lock, std::chrono::milliseconds(millsecond), [this] {return m_files.empty(); });
	}
}

void CRCScanner::Stop()
{
	{
		std::unique_lock<std::mutex> lock(m_mtx_file);
		m_bTerminate = true;
		m_cond.notify_all();
	}

	for (size_t i = 0; i < m_thread_pool.size(); i++)
	{
		if (m_thread_pool[i]->joinable())
		{
			m_thread_pool[i]->join();
		}
		delete m_thread_pool[i];
		m_thread_pool[i] = NULL;
	}

	std::unique_lock<std::mutex> lock(m_mtx_file);
	m_thread_pool.clear();
}

void
CRCScanner::ThreadScan(bool isQuick)
{
	if (!isQuick) {
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
	}
	else {
		while (!m_bTerminate)
		{
			fs::path path;
			bool ok = Get(path);
			if (ok) {
				++m_atomic;
				ScanFileQuick(path);
				--m_atomic;

				std::unique_lock<std::mutex> lock(m_mtx_file);
				if (m_atomic == 0 && m_files.empty())
				{
					//通知 waitforalldone
					m_cond.notify_all();
				}
			}
		}
	}
	m_wait++;

}

void
CRCScanner::Scan(const string& path)
{
	CRCLog::Info("Scan Start ");

	m_isQuick = false;

	time_t t = time(0);
	string filename = "XFullFileList";
	char tmp[32] = { 0 };
	strftime(tmp, sizeof(tmp), "-%Y%m%d %H%M00", localtime(&t));
	tmp[sizeof(tmp)-2] = '\0';
	filename.append(tmp);
	filename.append(".txt");

	m_outfile.open(filename.c_str());

	for (unsigned int i = 0; i < m_num_threads; i++) {
		m_thread_pool.push_back(new thread(&CRCScanner::ThreadScan, this, false));
	}

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

	m_thread_pool.clear();

	CRCLog::Info("Scan Complete ");
	CRCLog::Info("Scan File Amount: %d", m_total_file);

	//CRCLog::Info("Write File Label, Please Waiting... ");

	//string filestringlabel;
	//filestringlabel
	//	.append("SourceDir:").append(path);
	//{
	//	stringstream ss;
	//	ss << m_total_file;
	//	filestringlabel
	//		.append("\nTotalFilesCount:").append(ss.str());
	//}
	//{
	//	stringstream ss;
	//	ss << m_total_size;
	//	filestringlabel
	//		.append("\nTotalFileSize:").append(ss.str())
	//		.append("\n####");
	//}

	//InsertByLineNumber(filename, filestringlabel.c_str(), 1);

	//CRCLog::Info("Write File Complete. ");
}

void		
CRCScanner::ScanQuick(const string& path)
{
	CRCLog::Info("Quick Scan Start ");

	m_isQuick = true;

	if (!show_dot.empty()) { show_dot.clear(); }

	for (unsigned int i = 0; i < m_num_threads; i++) {
		m_thread_pool.push_back(new thread(&CRCScanner::ThreadScan, this, true));
	}

	ScanDirectoryQuick(path);

	thread* p = new thread(&CRCScanner::ThreadOutput, this);
	p->detach();

	WaitForAllDone();
	Stop();

	if (p->joinable()) {
		p->join();
	}

	CRCLog::Info("Scan Complete ");
	CRCLog::Info("Scan File Amount: %d", m_total_file);

	//CRCLog::Info("Write File Label, Please Waiting... ");

	//string filestringlabel;
	//filestringlabel
	//	.append("SourceDir:").append(path);
	//{
	//	stringstream ss;
	//	ss << m_total_file;
	//	filestringlabel
	//		.append("\nTotalFilesCount:").append(ss.str());
	//}
	//{
	//	stringstream ss;
	//	ss << m_total_size;
	//	filestringlabel
	//		.append("\nTotalFileSize:").append(ss.str())
	//		.append("\n####");
	//}

	//InsertByLineNumber(m_source_dir, filestringlabel.c_str(), 1);

	//CRCLog::Info("Write File Complete. ");
}

void
CRCScanner::ScanDirectoryQuick(fs::path directory)
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
			m_cond.notify_one();
		}
	}
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

void		
CRCScanner::Flush() {
	m_outfile.flush();
}