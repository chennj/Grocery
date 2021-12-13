#ifndef _SVR_MACHINE_CLIENT_H_
#define _SVR_MACHINE_CLIENT_H_

#include "crc_easy_select_client.h"
#include <queue>

class MachineClient : public CRCEasySelectClient
{
public:
	MachineClient();

	//响应网络消息
	virtual void OnNetMsg(CRCDataHeader* header);

	bool checkSend(time_t dt);

	void debug(int nSendSleep, int nMsg);

	void Loop();

	void Send2Stm32Loop();

	void PushSendMessage(SOCKET srcSock, CRCDataHeader* header);
	
	CRCDataHeader* popRecvMsg();

public:
	//发送时间计数
	time_t _tRestTime = 0;
private:
	//接收消息id计数
	int  _nRecvMsgID = 1;
	//发送消息id计数
	int  _nSendMsgID = 1;
	//发送条数计数
	int  _nSendCount = 0;
	//检查接收到的服务端消息ID是否连续
	bool _bCheckMsgID = false;
	//
	bool _bSend = false;
	//
	int  _nSendSleep = 1;
	//
	int  _nMsg = 1;
	//读写线程
	CRCThread _threadLoop;
	//消息发送线程
	CRCThread _threadSendCmd;
	//发送队列锁
	std::mutex _mutexSendMsg;
	//待发送队列
	std::queue<CRCDataHeader*> _machineSendMessages; 
	//接收队列锁
	std::mutex _mutexRecvMsg;
	//返回消息队列
	std::queue<CRCDataHeader*> _machineRecvMessages;
};

#endif