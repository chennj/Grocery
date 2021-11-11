#include "dirmgr.hpp"
#include <filesystem>
#include <iostream>
using namespace std;
namespace _NS_fs = std::filesystem;

void DirManager::check_dirs()
{
    try
    {
15       if (!(_NS_fs::exists("in")))
16          _NS_fs::create_directory("in");
17       if (!(_NS_fs::exists("out")))
18          _NS_fs::create_directory("out");
19    }
20    catch (const exception &e)
21    {
22       cout << "Error on trying to create [in]/[out] directories." << endl;
23       cerr << e.what() << endl;
24       exit(EXIT_FAILURE);
25    }
26 }

40 bool DirManager::start_watching(string path)
41 {
42    try
43    {
44       thread* obj_th = new thread(&DirManager::watching_task, this, path);
45       this->obj_linked_thd = obj_th;
46       return true;
47    }
48    catch (const std::exception &e)
49    {
50       std::cerr << e.what() << '\n';
51       return false;
52    }
53 }

6 DirManager::~DirManager()
7 {
8    this->obj_linked_thd->join();
9    delete this->obj_linked_thd;
10 }

27 void DirManager::watching_task(string path)
28 {
29    int files_count = this->count_items(path);
30    while (!this->stop_flag)
31    {
32       this_thread::sleep_for(chrono::seconds(3));
33       cout << "Monitoring [in] directory, interval 3 secs." << endl;
34       if (this->count_items(path) > files_count)
35          cout << "New file created." << endl;
36       files_count = this->count_items(path);
37    }
38    cout << "Monitor finished." << endl;
39 }

58 int DirManager::count_items(string path)
59 {
60    int items = 0;
61    for (auto& p : _NS_fs::directory_iterator(path))
62    {
63       if (_NS_fs::is_regular_file(p.path()))
64          items++;
65    }
66    return items;
67 }

