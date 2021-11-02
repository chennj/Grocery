#ifndef _CRC_TRANSFER_FILE_SERVER_H_
#define _CRC_TRANSFER_FILE_SERVER_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>

#define SOCKET int
#define INVALID_SOCKET	(SOCKET)(~0)
#define SOCKET_ERROR			(-1)
#endif

#include <stdio.h>
#include <memory>
#include <iostream>
#include <thread>

//运送数据的卡车
struct Truck {
	int id;					//数据的类型。1：文件数据，2：文件属性
	int index;				//数据块索引号
	char fileData[1024];	//数据块
							//使用1024的长度是因为一个UDP包的大小位1500字节
};

class CrcTransFileServer
{
private:
    SOCKET m_sock;
public:
    CrcTransFileServer();
    ~CrcTransFileServer();

private:
    unsigned long proc_recv();

public:
    int     Initialize();
    int     BindSocket(const char* ip, unsigned short port);
    void    Close();
    void    Start();
};
#endif //!_CRC_TRANSFER_FILE_SERVER_H_