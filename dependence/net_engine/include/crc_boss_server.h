#ifndef _CRC_TCPSERVER_H_
#define _CRC_TCPSERVER_H_
// file crc_tcpserver.h

#include "crc_common.h"
#include "crc_channel.h"
#include "crc_work_server.h"
#include "crc_i_net_event.h"
#include "crc_net_work.h"
#include "crc_config.h"

#include <thread>
#include <mutex>
#include <atomic>

class CRCBossServer : public CRCINetEvent
{
private:
	//
	CRCThread _thread;
	//消息处理对象，内部会创建线程
	std::vector<CRCWorkServer*> _workServers;
	//每秒消息计时
	CRCTimestamp _tTime;
	//
	SOCKET _sock;

protected:
	//发送至客户端数据缓冲区大小
	int _nSendBuffSize;
	//接收到客户端数据缓冲区大小
	int _nRecvBuffSize;
	//客户端连接上限
	int _nMaxClient;
	//SOCKET recv计数
	std::atomic_int _recvCount;
	//收到消息计数
	std::atomic_int _msgCount;
	//客户端计数
	std::atomic_int _clientAccept;
	//已分配客户端计数
	std::atomic_int _clientJoin;

public:
	CRCBossServer();
    virtual ~CRCBossServer();

public:
    //初始化Socket
	SOCKET InitSocket();
    //绑定IP和端口号
	int Bind(const char* ip, unsigned short port);
    //监听端口号
	int Listen(int n);
    //接受客户端连接
	SOCKET Accept();
    //将接入客户放入工作服务器
    void addClientToWorkServer(CRCChannel* pClient);
    //关闭Socket
	void Close();

    //crcWorkServer 多个线程触发 不安全
	//如果只开启1个 crcWorkServer 就是安全的
    //客户端接入
    virtual void OnNetJoin(CRCChannel* pClient);
    //crcWorkServer 多个线程触发 不安全
	//如果只开启1个 crcWorkServer 就是安全的
    //客户端离开
    virtual void OnNetLeave(CRCChannel* pClient);
    //处理消息计数
    virtual void OnNetMsg(CRCWorkServer* pServer, CRCChannel* pClient, CRCDataHeader* header);
	//接收消息计数
	virtual void OnNetRecv(CRCChannel* pClient);

	template<class TServer>
	void Start(int nWorkServer)
	{
		for (int n = 0; n < nWorkServer; n++)
		{
			auto ser = new TServer();
			ser->setId(n + 1);
			ser->setClientNum((_nMaxClient/nWorkServer)+1);
			_workServers.push_back(ser);
			//注册网络事件接受对象
			ser->setEventObj(this);
			//启动消息处理线程
			ser->Start();
		}
		_thread.Start(nullptr,
			[this](CRCThread* pThread) {
				OnRun(pThread);
			});
	}
	
protected:
	//处理网络消息
	virtual void OnRun(CRCThread* pThread) = 0;

	//计算并输出每秒收到的网络消息
	void time4msg();

	inline SOCKET sockfd(){return _sock;}
};

#endif