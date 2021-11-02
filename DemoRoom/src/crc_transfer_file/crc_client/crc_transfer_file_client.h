#ifndef _CRC_TRANSFER_FILE_CLIENT_H_
#define _CRC_TRANSFER_FILE_CLIENT_H_

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<Windows.h>
#include<WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
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

class CrcTransFileClient
{
private:
    SOCKET m_sock;
public:
    CrcTransFileClient();
    ~CrcTransFileClient();

private:
	//处理接收的消息
    unsigned long proc_recv();
	//获取文件的长度
	long get_file_len(const char* path);
	//从路径中获取文件名
	const char* get_file_name(const char* path);

public:
    int     Initialize();
    void    Close();
    void    StartTalking(const char* ip, unsigned short port);
};
#endif //!_CRC_TRANSFER_FILE_CLIENT_H_