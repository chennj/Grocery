#ifndef _CRC_COMMON_H_
#define _CRC_COMMON_H_
// file: crc_common.h

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#define _WINSOCK_DEPRECATED_NO_WARNINGS
	#include<windows.h>
	#include<WinSock2.h>
	#pragma comment(lib,"ws2_32.lib")
#else
#ifdef __APPLE__
    #define _DARWIN_UNLIMITED_SELECT
#endif // !__APPLE__
	#include<unistd.h> //uni std
	#include<arpa/inet.h>
	#include<string.h>
	#include<signal.h>
	#include<sys/socket.h>
	#include<netinet/tcp.h>

	#define SOCKET int
	#define INVALID_SOCKET  (SOCKET)(~0)
	#define SOCKET_ERROR            (-1)
#endif

#include "crc_message_header.h"
#include "crc_timestamp.h"
#include "crc_task_server.h"
#include "crc_log.h"

#include<stdio.h>

//缓冲区最小单元大小
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 8192
#define SEND_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE

#endif