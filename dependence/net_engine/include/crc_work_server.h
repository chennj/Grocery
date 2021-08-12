#ifndef _CRC_WORK_SERVER_H_
#define _CRC_WORK_SERVER_H_
// file: crc_work_server.h

#include "crc_common.h"
#include "crc_i_net_event.h"
#include "crc_channel.h"
#include "crc_semaphore.h"

#include <vector>
#include <map>

//网络消息接收处理服务类
class CRCWorkServer
{
private:
	//缓冲客户队列
	std::vector<CRCChannel*> _clientsBuff;
	//缓冲队列的锁
	std::mutex _mutex;
	//网络事件对象
	CRCINetEvent* _pNetEvent = nullptr;
	//
	CRCTaskServer _taskServer;
	//旧的时间戳
	time_t _oldTime = CRCTime::getNowInMilliSec();
	//
	CRCThread _thread;
protected:
	//
	int _id = -1;
	//客户列表是否有变化
	bool _clients_change = true;
    //正式客户队列
	std::map<SOCKET, CRCChannel*> _clients;

public:
	virtual ~CRCWorkServer();

private:
	void ClearClients();

public:
	virtual void setClientNum(int nSocketNum);
	virtual bool DoNetEvents() = 0;
	virtual void OnClientJoin(CRCChannel* pClient);
	//响应网络消息
	virtual void OnNetMsg(CRCChannel* pClient, CRCDataHeader* header);

	void setId(int id);
	void setEventObj(CRCINetEvent* event);
	//关闭Socket
	void Close();
	//处理网络消息
	void OnRun(CRCThread* pThread);
	void CheckTime();
	void OnClientLeave(CRCChannel* pClient);
	void OnNetRecv(CRCChannel* pClient);
	void DoMsg();
	//接收数据 处理粘包 拆分包
	int RecvData(CRCChannel* pClient);
	void addClient(CRCChannel* pClient);
	void Start();
	size_t getClientCount();
};

#endif