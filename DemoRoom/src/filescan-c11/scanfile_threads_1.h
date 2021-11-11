#include <string>
2  #include <thread>
3  using namespace std;
4  class DirManager
5  {
6      private:
7          void watching_task(string path);
8          bool stop_flag = false;
9          int count_items(string path);
10         thread* obj_linked_thd;
11      public:
12          ~DirManager();
13          static void check_dirs();
14          bool start_watching(string path);
15          void stop_watching();
16  };