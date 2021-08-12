#include "../../include/crc_work_server.h"

CRCWorkServer::~CRCWorkServer()
{
    CRCLog_Info("WorkServer%d.~WorkServer exit begin", _id);
    Close();
    CRCLog_Info("WorkServer%d.~WorkServer exit end", _id);
}

void
CRCWorkServer::ClearClients()
{
    for (auto iter : _clients)
    {
        delete iter.second;
    }
    _clients.clear();

    for (auto iter : _clientsBuff)
    {
        delete iter;
    }
    _clientsBuff.clear();
}

void
CRCWorkServer::setId(int id)
{
    _id = id;
    _taskServer.serverId = id;    
}

void
CRCWorkServer::setClientNum(int nSocketNum)
{

}

void
CRCWorkServer::setEventObj(CRCINetEvent* event)
{
    _pNetEvent = event;
}

void
CRCWorkServer::Close()
{
    CRCLog_Info("WorkServer%d.Close begin", _id);
    _taskServer.Close();
    _thread.Close();
    CRCLog_Info("WorkServer%d.Close end", _id);    
}

void
CRCWorkServer::OnRun(CRCThread* pThread)
{
    while (pThread->isRun())
    {
        if (!_clientsBuff.empty())
        {
            //从缓冲队列里取出客户数据
            std::lock_guard<std::mutex> lock(_mutex);
            for (auto pClient : _clientsBuff)
            {
                _clients[pClient->sockfd()] = pClient;
                pClient->serverId = _id;
                if (_pNetEvent)
                    _pNetEvent->OnNetJoin(pClient);
                OnClientJoin(pClient);
            }
            _clientsBuff.clear();
            _clients_change = true;
        }

        if (_clients.empty())
        {
            //如果没有需要处理的客户端，就跳过
            CRCThread::Sleep(1);
            //旧的时间戳
            _oldTime = CRCTime::getNowInMilliSec();
            continue;
        }

        CheckTime();
        if (!DoNetEvents())
        {
            pThread->Exit();
            break;
        }
        DoMsg();
    }
    CRCLog_Info("WorkServer%d.OnRun exit", _id);
}

void
CRCWorkServer::CheckTime()
{
    //当前时间戳
    auto nowTime = CRCTime::getNowInMilliSec();
    auto dt = nowTime - _oldTime;
    _oldTime = nowTime;

    CRCChannel* pClient = nullptr;
    for (auto iter = _clients.begin(); iter != _clients.end(); )
    {
        pClient = iter->second;
        //心跳检测
        if (pClient->checkHeart(dt))
        {
#ifdef CRC_USE_IOCP
            if(pClient->isPostIoAction())
                pClient->destory();
            else
                OnClientLeave(pClient);
#else
            OnClientLeave(pClient);
#endif // CRC_USE_IOCP
            iter = _clients.erase(iter);
            continue;
        }

        ////定时发送检测
        //pClient->checkSend(dt);

        iter++;
    }    
}

void
CRCWorkServer::OnClientLeave(CRCChannel* pClient)
{
    if (_pNetEvent)
        _pNetEvent->OnNetLeave(pClient);
    _clients_change = true;
    delete pClient;
}

void
CRCWorkServer::OnClientJoin(CRCChannel* pClient)
{

}

void
CRCWorkServer::OnNetRecv(CRCChannel* pClient)
{
    if (_pNetEvent)
        _pNetEvent->OnNetRecv(pClient);
}

void
CRCWorkServer::DoMsg()
{
    CRCChannel* pClient = nullptr;
    for (auto itr : _clients)
    {
        pClient = itr.second;
        //循环 判断是否有消息需要处理
        while (pClient->hasMsg())
        {
            //处理网络消息
            OnNetMsg(pClient, pClient->front_msg());
            //移除消息队列（缓冲区）最前的一条数据
            pClient->pop_front_msg();
        }
    }    
}

int
CRCWorkServer::RecvData(CRCChannel* pClient)
{
    //接收客户端数据
    int nLen = pClient->RecvData();
    //触发<接收到网络数据>事件
    if (_pNetEvent)
        _pNetEvent->OnNetRecv(pClient);
    return nLen;
}

void
CRCWorkServer::OnNetMsg(CRCChannel* pClient, CRCDataHeader* header)
{
    if (_pNetEvent)
        _pNetEvent->OnNetMsg(this, pClient, header);
}

void
CRCWorkServer::addClient(CRCChannel* pClient)
{
    std::lock_guard<std::mutex> lock(_mutex);
    //_mutex.lock();
    _clientsBuff.push_back(pClient);
    //_mutex.unlock();
}

void
CRCWorkServer::Start()
{
    _taskServer.Start();
    _thread.Start(
        //onCreate
        nullptr,
        //onRun
        [this](CRCThread* pThread) {
            OnRun(pThread);
        },
        //onDestory
        [this](CRCThread* pThread) {
            ClearClients();
        }
    );    
}

size_t
CRCWorkServer::getClientCount()
{
    return _clients.size() + _clientsBuff.size();
}