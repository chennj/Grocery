#ifndef _CRC_EASY_EPOLL_CLIENT_H_
#define _CRC_EASY_EPOLL_CLIENT_H_

#if __linux__
#include "crc_easy_tcp_client.h"
#include "crc_epoll.h"

class CRCEasyEpollClient : public CRCEasyTcpClient
{
public:
    virtual void OnInitSocket() override;

    virtual void Close() override;

    //处理网络消息
    bool OnRun(int microseconds = 1) override;
    
protected:
    CRCEpoll _ep;
};

#endif
#endif //!_CRC_EASY_EPOLL_CLIENT_H_