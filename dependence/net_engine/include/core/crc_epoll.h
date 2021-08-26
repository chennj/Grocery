#ifndef _CRC_EPOLL_H_
#define _CRC_EPOLL_H_

#if __linux__
//----------
#include "crc_common.h"
#include "crc_channel.h"
#include "crc_log.h"
#include <sys/epoll.h>
#define EPOLL_ERROR            (-1)
//----------

class CRCEpoll
{
private:
	//用于接收检测到的网络事件的数组
	epoll_event* _pEvents = nullptr;
	//
	int _nMaxEvents = 1;
	//
	int _epfd = -1;
    
public:
	~CRCEpoll();

public:
	int create(int nMaxEvents);

	void destory();

	//向epoll对象注册需要管理、监听的Socket文件描述符
	int ctl(int op, SOCKET sockfd, uint32_t events);

	//向epoll对象注册需要管理、监听的Socket文件描述符
	int ctl(int op, CRCChannel* pClient, uint32_t events);

	int wait(int timeout);

	inline epoll_event* events()
	{
		return _pEvents;
	}
};

#endif //__linux__

#endif // !_CRC_EPOLL_H_