/**
 * 
 * author:  chenningjiang
 * desc:    tcp协议的接收服务端基类
 * 
 * */
#ifndef _CRC_EASY_TCP_SERVER_H_
#define _CRC_EASY_TCP_SERVER_H_

#include "crc_base.h"
#include "crc_client.h"
#include "crc_work_server.h"
#include "crc_i_net_event.h"
#include "crc_net_work.h"
#include "crc_config.h"
		 
#include <thread>
#include <mutex>
#include <atomic>

class CRCEasyTcpServer : public CRCINetEvent
{
private:
    //网络事件处理线程
    CRCThread _thread;
    //消息处理对象，其内部会创建线程
    std::vector<CRCWorkServer*> _workServers;
    //每秒消息计时，统计用
    CRCTimestamp _tTime;
    //套接字
    SOCKET _sock;

protected:
    //
    int _address_family = AF_INET;
    //客户端发送缓冲区大小
    int _nSendBuffSize;
    //客户端接收缓冲区大小
    int _nRecvBuffSize;
    //客户端连接上限
    int _nMaxClient;
    //接收计数
    std::atomic_int _recvCount;
    //接收完整消息计数
    std::atomic_int _msgCount;
    //客户端计数
    std::atomic_int _clientAccept;
    //已分配客户端计数
    std::atomic_int _clientJoin;

public:
    CRCEasyTcpServer();
    virtual ~CRCEasyTcpServer();

public:
    //初始化SOCKET
    SOCKET InitSocket(int af = AF_INET);
    
    //绑定IP和端口号
    int Bind(const char* ip, unsigned short port);

    //监听端口
    int Listen(int n);

    //接受客户端连接
    SOCKET Accept();

    //将新进入的客户端加入 CRCWorkServer
    void AddClientToCRCWorkServer(CRCClient* pClient);

    //关闭SOCKET
    void Close();

    CRCClient* find_client(int id);

    //加入事件
    virtual void OnNetJoin(CRCClient* pClient) override;

    //离开事件
    virtual void OnNetLeave(CRCClient* pClient) override;

    //消息事件
    virtual void OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* pHeader) override;

    //recv事件
    virtual void OnNetRecv(CRCClient* pClient) override;

    //
    virtual CRCClient* makeClientObj(SOCKET cSock);

protected:
    //处理网络消息
    virtual void OnRun(CRCThread* pThread) = 0;

    //计算并输出每秒收到的网络消息
    void time4msg();

    inline SOCKET sockfd(){return _sock;} 

    inline std::vector<CRCWorkServer*>& workServers(){return _workServers;}

public:
    //启动CRCWorkServer服务器
    template<class ServerT>
    void Start(int nCRCWorkServer)
    {
        for (int i=0; i<nCRCWorkServer; i++)
        {
            auto server = new ServerT();
            //设置 CRCWorkServer 服务器id,主要用于统计调试
            server->setId(i+1);
            //设置每个服务器最大可处理客户端数目，暂时未使用
            server->setClientNum((_nMaxClient/nCRCWorkServer)+1);
            //加入工作服务器队列
            _workServers.push_back(server);
            //注册网络事件接收对象
            server->setEventObj(this);
            //启动服务器
            server->Start();
        }

        _thread.Start(
            nullptr,
            [this](CRCThread* pThread)
            {
                OnRun(pThread);
            }
        );
    }

public:
    SOCKET Accept_IPv6();
    
    SOCKET Accept_IPv4();

    void AcceptClient(SOCKET cSock, char* ip);

};

#endif  //!_CRC_EASY_TCP_SERVER_H_