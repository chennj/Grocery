#include "crc_easy_epoll_client.h"

void 
CRCEasyEpollClient::OnInitSocket()
{
    _ep.create(1);
    _ep.ctl(EPOLL_CTL_ADD, _pClient, EPOLLIN);
};

void
CRCEasyEpollClient::Close()
{
    _ep.destory();
    CRCEasyTcpClient::Close();
}

bool 
CRCEasyEpollClient::OnRun(int microseconds)
{
    if (isRun())
    {
        if (_pClient->needWrite())
        {
            _ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN | EPOLLOUT);
        }
        else {
            _ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN);
        }
        //---
        int ret = _ep.wait(microseconds);
        if (ret < 0)
        {
            CRCLog_Error("CELLEpollClient.OnRun.wait clientId<%d> sockfd<%d>", _pClient->id, (int)_pClient->sockfd());
            return false;
        }
        else if (ret == 0)
        {
            return true;
        }
        //---
        auto events = _ep.events();
        for (int i = 0; i < ret; i++)
        {
            CRCClient* pClient = (CRCClient*)events[i].data.ptr;
            //当服务端socket发生事件时，表示有新客户端连接
            if (pClient)
            {
                if (events[i].events & EPOLLIN)
                {
                    
                    if (SOCKET_ERROR == RecvData())
                    {
                        CRCLog_Error("<socket=%d>OnRun.epoll RecvData exit", pClient->sockfd());
                        Close();
                        continue;
                    }
                }
                if (events[i].events & EPOLLOUT)
                {
                    if (SOCKET_ERROR == pClient->SendDataReal())
                    {
                        CRCLog_Error("<socket=%d>OnRun.epoll SendDataReal exit", pClient->sockfd());
                        Close();
                    }
                }
            }
        }
        return true;
    }
    return false;
}