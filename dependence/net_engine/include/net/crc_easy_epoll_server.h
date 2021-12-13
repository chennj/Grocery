#ifndef _CRC_EASY_EPOLL_SERVER_H_
#define _CRC_EASY_EPOLL_SERVER_H_

#if __linux__
#include "crc_easy_tcp_server.h"
#include "crc_work_epoll_server.h"
#include "crc_epoll.h"

class CRCEasyEpollServer : public CRCEasyTcpServer
{
public:
    void Start(int nWorkServer);
protected:
    //处理网络消息
    void OnRun(CRCThread* pThread);
};

#endif
#endif //!_CRC_EASY_EPOLL_SERVER_H_