#ifndef _CRC_WORK_EPOLL_SERVER_H_
#define _CRC_WORK_EPOLL_SERVER_H_

#include "crc_work_server.h"
#include "crc_epoll.h"

//网络消息接收处理服务类
class CRCWorkEpollServer : public CRCWorkServer
{
private:
	CRCEpoll _ep;

public:
	CRCWorkEpollServer();
	~CRCWorkEpollServer() noexcept;

public:
	virtual void setClientNum(int nSocketNum);

	bool DoNetEvents();

	void rmClient(CRCChannel* pClient);

	void OnClientJoin(CRCChannel* pClient);
};

#endif // !_CRC_WORK_EPOLL_SERVER_H_