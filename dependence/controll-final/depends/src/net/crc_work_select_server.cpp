#include "crc_work_select_server.h"

CRCWorkSelectServer::~CRCWorkSelectServer()
{
    Close();
}

void 
CRCWorkSelectServer::setClientNum(int nSocketNum)
{
    _fdRead.create(nSocketNum);
    _fdWrite.create(nSocketNum);
    _fdRead_bak.create(nSocketNum);
}

bool 
CRCWorkSelectServer::DoNetEvents()
{
    //计算可读集合
    if (_clients_change)
    {
        _clients_change = false;
        //清理集合
        _fdRead.zero();
        //将描述符（socket）加入集合
        _maxSock = _clients.begin()->second->sockfd();
        for (auto iter : _clients)
        {
            _fdRead.add(iter.second->sockfd());
            if (_maxSock < iter.second->sockfd())
            {
                _maxSock = iter.second->sockfd();
            }
        }
        _fdRead_bak.copy(_fdRead);
    }
    else {
        _fdRead.copy(_fdRead_bak);
    }
    //计算可写集合
    bool bNeedWrite = false;
    _fdWrite.zero();
    for (auto iter : _clients)
    {	//需要写数据的客户端,才加入fd_set检测是否可写
        if (iter.second->needWrite())
        {
            bNeedWrite = true;
            _fdWrite.add(iter.second->sockfd());
        }
    }

    ///nfds 是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
    ///既是所有文件描述符最大值+1 在Windows中这个参数可以写0
    timeval t{ 0,1 };
    int ret = 0;
    if (bNeedWrite)
    {
        ret = select(_maxSock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
    }
    else {
        ret = select(_maxSock + 1, _fdRead.fdset(), nullptr, nullptr, &t);
    }
    if (ret < 0)
    {
        if(errno == EINTR)
        {
            CRCLog_Info("CRCWorkSelectServer select EINTR");
            return true;
        }
        CRCLog_PError("CRCWorkSelectServer%d.OnRun.select", _id);
        return false;
    }
    else if (ret == 0)
    {
        return true;
    }
    ReadData();
    WriteData();
    return true;
}

void 
CRCWorkSelectServer::WriteData()
{
#ifdef _WIN32
    auto pfdset = _fdWrite.fdset();
    for (int n = 0; n < pfdset->fd_count; n++)
    {
        auto iter = _clients.find(pfdset->fd_array[n]);
        if (iter != _clients.end())
        {
            if (SOCKET_ERROR == iter->second->SendDataReal())
            {
                OnClientLeave(iter->second);
                _clients.erase(iter);
            }
        }
    }
    
#else
    for (auto iter = _clients.begin(); iter != _clients.end(); )
    {
        if (iter->second->needWrite() && _fdWrite.has(iter->second->sockfd()))
        {
            if (SOCKET_ERROR == iter->second->SendDataReal())
            {
                OnClientLeave(iter->second);
                auto iterOld = iter;
                iter++;
                _clients.erase(iterOld);
                continue;
            }
        }
        iter++;
    }
#endif
}

void 
CRCWorkSelectServer::ReadData()
{
#ifdef _WIN32
    auto pfdset = _fdRead.fdset();
    for (int n = 0; n < pfdset->fd_count; n++)
    {
        auto iter = _clients.find(pfdset->fd_array[n]);
        if (iter != _clients.end())
        {
            if (SOCKET_ERROR == RecvData(iter->second))
            {
                OnClientLeave(iter->second);
                _clients.erase(iter);
            }
        }
    }
#else
    for (auto iter = _clients.begin(); iter != _clients.end(); )
    {
        if (_fdRead.has(iter->second->sockfd()))
        {
            if (SOCKET_ERROR == RecvData(iter->second))
            {
                OnClientLeave(iter->second);
                auto iterOld = iter;
                iter++;
                _clients.erase(iterOld);
                continue;
            }
        }
        iter++;
    }
#endif
}