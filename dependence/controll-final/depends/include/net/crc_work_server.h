/**
 * 
 * author:  chenningjiang
 * desc:    工作线程基类
 * 
 * */
#ifndef _CRC_WORK_SERVER_H_
#define _CRC_WORK_SERVER_H_

#include "crc_base.h"
#include "crc_i_net_event.h"
#include "crc_client.h"
#include "crc_semaphore.h"

#include <vector>
#include <map>

//网络消息接收处理服务类
class CRCWorkServer
{
public:
	virtual ~CRCWorkServer();

	void setId(int id);

	void setEventObj(CRCINetEvent* event);

	//关闭Socket
	void Close();

	//处理网络消息
	void OnRun(CRCThread* pThread);

	void CheckTime();

	void OnClientLeave(CRCClient* pClient);

	void OnNetRecv(CRCClient* pClient);

	void DoMsg();

	//接收数据 处理粘包 拆分包
	int  RecvData(CRCClient* pClient);

	void addClient(CRCClient* pClient);

	void Start();

	size_t getClientCount();

	CRCClient* find_client(int id);

	virtual bool DoNetEvents() = 0;

	virtual void setClientNum(int nSocketNum);

	virtual void OnClientJoin(CRCClient* pClient);

	//响应网络消息
	virtual void OnNetMsg(CRCClient* pClient, CRCDataHeader* header);

	//void addSendTask(CRCClient* pClient, CRCDataHeader* header)
	//{
	//	_taskServer.addTask([pClient, header]() {
	//		pClient->SendData(header);
	//		delete header;
	//	});
	//}
    
private:
	void ClearClients();

protected:
	//正式客户队列
	std::map<SOCKET, CRCClient*> _clients;
private:
	//缓冲客户队列
	std::vector<CRCClient*> _clientsBuff;
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

public:
	inline const std::map<SOCKET, CRCClient*>& clients() const {return _clients;}
};

#endif // !_CRC_WORK_SERVER_H_
