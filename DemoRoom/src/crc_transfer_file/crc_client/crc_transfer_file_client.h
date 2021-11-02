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

//�������ݵĿ���
struct Truck {
	int id;					//���ݵ����͡�1���ļ����ݣ�2���ļ�����
	int index;				//���ݿ�������
	char fileData[1024];	//���ݿ�
							//ʹ��1024�ĳ�������Ϊһ��UDP���Ĵ�Сλ1500�ֽ�
};

class CrcTransFileClient
{
private:
    SOCKET m_sock;
public:
    CrcTransFileClient();
    ~CrcTransFileClient();

private:
	//������յ���Ϣ
    unsigned long proc_recv();
	//��ȡ�ļ��ĳ���
	long get_file_len(const char* path);
	//��·���л�ȡ�ļ���
	const char* get_file_name(const char* path);

public:
    int     Initialize();
    void    Close();
    void    StartTalking(const char* ip, unsigned short port);
};
#endif //!_CRC_TRANSFER_FILE_CLIENT_H_