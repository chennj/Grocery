1 #include "dirmgr.hpp"
2 #include <iostream>
3 using namespace std;
4 int main()
5 {
6    DirManager::check_dirs();
7    DirManager obj_dm;
8    if (!obj_dm.start_watching("./in"))
9    {
10       cout << "Could not start Directory Monitor." << endl;
11       return EXIT_FAILURE;
12    }
13    for (int n = 0; n <= 20; n++)
14    {
15       this_thread::sleep_for(chrono::seconds(2));
16       cout << "Main thread alive." << endl;
17    }
18    obj_dm.stop_watching();
19    cout << "Main thread finished." << endl;
20    return EXIT_SUCCESS;
21 }