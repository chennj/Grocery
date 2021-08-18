#include "../../include/crc_boss_epoll_server.h"

void
CRCBossEpollServer::Start(int nWorkServer)
{
    CRCBossServer::Start<CRCWorkEpollServer>(nWorkServer);
}

void
CRCBossEpollServer::OnRun(CRCThread* pThread)
{
    CRCEpoll ep;
    ep.create(_nMaxClient);
    ep.ctl(EPOLL_CTL_ADD, sockfd(), EPOLLIN);
    while (pThread->isRun())
    {
        time4msg();
        //---
        int ret = ep.wait(1);
        if (ret < 0)
        {
            CRCLog_Error("CRCBossEpollServer.OnRun ep.wait exit.");
            pThread->Exit();
            break;
        }
        //---
        auto events = ep.events();
        for(int i = 0; i < ret; i++)
        {
            //当服务端socket发生事件时，表示有新客户端连接
            if(events[i].data.fd == sockfd())
            {
                if(events[i].events & EPOLLIN)
                {
                    Accept();
                }
            }
        }
    }    
}