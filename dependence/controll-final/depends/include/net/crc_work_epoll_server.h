/**
 * 
 * author:  chenningjiang
 * desc:    接收服务端epoll模型的工作线程
 * 
 * */
#ifndef _CRC_WORK_EPOLL_SERVER_H_
#define _CRC_WORK_EPOLL_SERVER_H_

#if __linux__
#include "crc_work_server.h"
#include "crc_epoll.h"

class CRCWorkEpollServer : public CRCWorkServer
{
public:
	CRCWorkEpollServer();

	~CRCWorkEpollServer() noexcept;

	virtual void setClientNum(int nSocketNum);

	bool DoNetEvents();

	void rmClient(CRCClient* pClient);

	void OnClientJoin(CRCClient* pClient);
	
private:
	CRCEpoll _ep;
};

#endif 

#endif //!_CRC_WORK_EPOLL_SERVER_H_