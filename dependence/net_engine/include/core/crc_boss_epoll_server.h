#ifndef _CRC_BOSS_EPOLL_SERVER_HPP_
#define _CRC_BOSS_EPOLL_SERVER_HPP_

#include "crc_boss_server.h"
#include "crc_work_epoll_server.h"
#include "crc_epoll.h"

class CRCBossEpollServer : public CRCBossServer
{
public:
	void Start(int nWorkServer);
protected:
	//处理网络消息
	void OnRun(CRCThread* pThread);
};

#endif //!_CRC_BOSS_EPOLL_SERVER_HPP_