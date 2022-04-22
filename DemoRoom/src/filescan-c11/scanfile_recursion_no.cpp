//g++ -o scanfile scanfile.cpp
#if __LINUX__
#include <iostream>
#include <string>
#include <vector>
#include <regex.h>
#include <assert.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
using namespace std;
struct scan_info    //扫描文件信息
{
    string file_dir;
    string file_name;
    int create_time;
};
class compare_name    //lhs > rhs,get file in ascending order.
{
public:
    /*Sort by file creation time and file_name in descending order, so get file in back will be in ascending order.*/
    bool operator()(const scan_info& lhs, const scan_info& rhs) {
        if (lhs.file_name > rhs.file_name)  return true;
        //else if (lhs.create_time == rhs.create_time && lhs.file_name > rhs.file_name)  return true;
        else  return false;
    }
};
template <typename compare = compare_name>
class scan_file
{
public:
    // Scan file in single-directory mode.
    scan_file(const string& file_dir, const string& pattern, int file_count = 1024);
    // Scan file in multi-directory mode.
    scan_file(const vector<string>& dir_vector, const string& pattern, int file_count = 1024);
    // Scan file in dir/sub-dirs mode.
    scan_file(const string& dir, const vector<string>& sub_dirs, const string& pattern, int file_count = 1024);
    virtual ~scan_file();
public:
    // Get a file in given directories. Upon file found, return true, otherwise return false.
    // In single-directory mode, return file name, otherwise return full name.
    bool get_file(string& file_name);
    // Get all files in given directories.
    // In single-directory mode, return file name, otherwise return full name.
    void get_files(vector<string>& files);
private:
    vector<string> dir_vector;
    regex_t reg;
    vector<scan_info> file_vector;
};
template<typename compare> scan_file<compare>::scan_file(const string& file_dir, const string& pattern, int file_count)
    : dir_vector(1, file_dir)
{
    assert(regcomp(&reg, pattern.c_str(), REG_NOSUB | REG_EXTENDED) == 0);
    file_vector.reserve(file_count);
}
template<typename compare> scan_file<compare>::scan_file(const vector<string>& dir_vector_, const string& pattern, int file_count)
    : dir_vector(dir_vector_)
{
    // 以功能更加强大的扩展正则表达式的方式进行匹配,不用存储匹配后的结果
    assert(regcomp(&reg, pattern.c_str(), REG_NOSUB | REG_EXTENDED) == 0);
    file_vector.reserve(file_count);
}
template<typename compare> scan_file<compare>::scan_file(const string& dir, const vector<string>& sub_dirs, const string& pattern, int file_count)
{
    vector<string>::const_iterator iter;
    for (iter = sub_dirs.begin(); iter != sub_dirs.end(); ++iter) {
        dir_vector.push_back(dir + '/' + *iter);
    }
    assert(regcomp(&reg, pattern.c_str(), REG_NOSUB | REG_EXTENDED) == 0);
    file_vector.reserve(file_count);
}
template<typename compare> scan_file<compare>::~scan_file()
{
    regfree(&reg);
}
template<typename compare> bool scan_file<compare>::get_file(string& file_name)
{
    /**先扫描目录,将所有的文件都写入到vector中**/
    /**如果找到文件,每次从vector中读取一个文件**/
    /**不能递归扫描,多文件时会返回全路径**/
    DIR* dirp;
    dirent ent;
    dirent* result;
    struct stat stat_buf;
    string full_name = "";
    scan_info file_info;
    file_name = "";
    while (file_vector.size() > 0) {
        vector<scan_info>::iterator iter = file_vector.begin();
        if (access((iter->file_dir + '/' + iter->file_name).c_str(), F_OK) == -1) {
            std::pop_heap(file_vector.begin(), file_vector.end(), compare());
            file_vector.pop_back();
            continue;
        }
        if (dir_vector.size() == 1)  file_name = iter->file_name;
        else  file_name = iter->file_dir + '/' + iter->file_name;
        std::pop_heap(file_vector.begin(), file_vector.end(), compare());
        file_vector.pop_back();
        return true;
    }
    vector<string>::const_iterator dir_iter;
    for (dir_iter = dir_vector.begin(); dir_iter != dir_vector.end(); ++dir_iter) {
        assert((dirp = opendir(dir_iter->c_str())) != NULL);
        while (readdir_r(dirp, &ent, &result) == 0 && result != 0) {
            if (strcmp(ent.d_name, ".") == 0 || strcmp(ent.d_name, "..") == 0)  continue;
            if (regexec(&reg, ent.d_name, (size_t)0, 0, 0) != 0)  continue;
            full_name = *dir_iter + '/' + ent.d_name;
            assert(::lstat(full_name.c_str(), &stat_buf) >= 0);
            if (S_ISDIR(stat_buf.st_mode) == 0) {
                file_info.file_dir = *dir_iter;
                file_info.file_name = ent.d_name;
                file_info.create_time = stat_buf.st_mtime;
                file_vector.push_back(file_info);
            }
        }
        closedir(dirp);
    }
    /**也可以采用文件加载完毕后更改名字
    *err_msg << "mv " << m_real_file << " " << m_real_file << ".bak";
    *system(err_msg.str().c_str());
    **/
    if (dir_vector.size() > 0)  dir_vector.clear();
    if (file_vector.size() > 0) {
        //make_heap以迭代器[start,end] 区间内的元素生成一个堆. 默认使用元素类型 的 < 操作符 进行判断堆的类型, 因此生成的是大顶堆. 这里是小顶堆
        std::make_heap(file_vector.begin(), file_vector.end(), compare());
        while (file_vector.size() > 0) {
            vector<scan_info>::iterator iter = file_vector.begin();
            if (access((iter->file_dir + '/' + iter->file_name).c_str(), F_OK) == -1) { //文件不存在
                //pop_heap() 并不是真的把最大（最小）的元素从堆中弹出来. 而是重新排序堆. 它把首元素和末元素交换，然后将[first,last-1]的数据再做成一个堆。
                std::pop_heap(file_vector.begin(), file_vector.end(), compare());
                file_vector.pop_back();
                continue;
            }
            if (dir_vector.size() == 1)  file_name = iter->file_name;
            else  file_name = iter->file_dir + '/' + iter->file_name;
            std::pop_heap(file_vector.begin(), file_vector.end(), compare());
            file_vector.pop_back();
            return true;
        }
 
        return false;
    }
    else {
        return false;
    }
}
template<typename compare> void scan_file<compare>::get_files(vector<string>& files)
{
    /**只扫描该目录下的文件,不扫描文件夹**/
    /**若想递归扫描,可将每次扫描到的文件push_back进vector**/
    DIR* dirp;
    dirent ent;
    dirent* result;
    struct stat stat_buf;
    string full_name = "";
    files.resize(0);
    vector<string>::const_iterator dir_iter;
    for (dir_iter = dir_vector.begin(); dir_iter != dir_vector.end(); ++dir_iter) {
        assert((dirp = opendir(dir_iter->c_str())) != NULL);
        while (readdir_r(dirp, &ent, &result) == 0 && result != 0) {
            if (strcmp(ent.d_name, ".") == 0 || strcmp(ent.d_name, "..") == 0)  continue;
            full_name = *dir_iter + '/' + ent.d_name;
            if (regexec(&reg, ent.d_name, (size_t)0, 0, 0) != 0)  continue;
            assert(::lstat(full_name.c_str(), &stat_buf) >= 0);
            if (S_ISDIR(stat_buf.st_mode) == 0) {        //不是文件夹
                if (regexec(&reg, ent.d_name, (size_t)0, 0, 0) == 0) {
                    files.push_back(ent.d_name);
                }
            }
        }
        closedir(dirp);
    }
}
int main()
{
    string path = "/tmp/other";
    string pattern = ".*.cpp";
    scan_file<> *tmp = new scan_file<>(path, pattern, 1);
    /**********方式一:单个文件获取************/
    string file = "";
    while (tmp->get_file(file) == true) {
        cout<<file<<endl;
    }
    delete tmp;
    /**********方式二:vecotor获取************/
    cout<<"------------------------------------"<<endl;
    tmp = new scan_file<>(path, pattern, 1);
    vector<string> files;
    tmp->get_files(files);
    vector<string>::iterator it;
    for ( it = files.begin(); it < files.end(); it++ ) {
        cout<<*it<<endl;
    }
    delete tmp;
    return 0;
}
#endif