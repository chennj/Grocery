/***************************************************** 
** Name         : main.cpp
** Author       : cnj
** Version      : 1.0 
** Date         : 2021-11-11
** Description  : 多线程文件扫描
******************************************************/ 
//g++ complier command
//debug
//g++ -g -std=c++11 -o scanfile main.cpp scanfile_threads.cpp -lpthread -lstdc++fs
//release
//g++ -std=c++11 -o scanfile main.cpp scanfile_threads.cpp -lpthread -lstdc++fs
//static
//g++ -g -static -std=c++11 -o scanfile main.cpp scanfile_threads.cpp -lpthread -lstdc++fs
//g++ -static -std=c++11 -o scanfile main.cpp scanfile_threads.cpp -lpthread -lstdc++fs
//gdb
//gdb -q --args ./scanfile /home/chennj/mycode/srs-rtmp-server/srs-0.1.0
//usage:
//./scanfile /home/chennj/mycode/srs-rtmp-server/srs-0.1.0 
//./scanfile /home/chennj/mycode/srs-rtmp-server/srs-0.1.0 10
#include "scanfile_threads.h"
#include <chrono>


int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cout << "===================================" << std::endl;
		std::cout << "usage : " << std::endl;
		std::cout << "[sudo]./scanfile scandir 1|0(whether need crc) [threads amount]" << std::endl;
        std::cout << "===================================" << std::endl;
		return 0;
	}

	CRCScanner Scanner(argc==4 ? atoi(argv[3]) : 0);
	Scanner.SetNeedCRC(atoi(argv[2])==1);
	std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();
	Scanner.Scan(argv[1]);
	auto count = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - begin).count() * 0.000001;
	std::cout << "Elapsed Time: " << count << "(s)" << std::endl;

    return 0;
}