/**
 * 
 * author:  chenningjiang
 * desc:    tcp协议的连接客户端基类
 * 
 * */
#ifndef _CRC_EASY_TCP_CLIENT_H_
#define _CRC_EASY_TCP_CLIENT_H_

#include "crc_base.h"

#include "crc_net_work.h"
#include "crc_msg_header.h"
#include "crc_client.h"

class CRCEasyTcpClient
{
public:
	CRCEasyTcpClient();

	virtual ~CRCEasyTcpClient();

	//初始化socket
	SOCKET InitSocket(int af,int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);

	//连接服务器
	int Connect(const char* ip, unsigned short port);

	//关闭套节字closesocket
	virtual void Close();

	//并非立即关闭socket
	//并非立即释放Client对象
	//而是将Client的状态设置为clientState_close
	void toClose();

	//处理网络消息
	virtual bool OnRun(int microseconds = 1) = 0;

	//是否工作中
	bool isRun();

	//接收数据 处理粘包 拆分包
	int RecvData();

	void DoMsg();

	//响应网络消息
	virtual void OnNetMsg(CRCDataHeader* header) = 0;

	//发送数据
	int SendData(CRCDataHeader* header);

	int SendData(const char* pData, int len);

	void setScopeIdName(std::string scope_id_name);

protected:
	virtual CRCClient* makeClientObj(SOCKET cSock, int sendSize, int recvSize);

	virtual void OnInitSocket();

	virtual void OnConnect();

	virtual void OnDisconnect();
	
protected:
	CRCClient * _pClient = nullptr;
	int _address_family = AF_INET;
	std::string _scope_id_name;
	bool _isConnect = false;
	volatile bool _stopRead = false;
};

#endif