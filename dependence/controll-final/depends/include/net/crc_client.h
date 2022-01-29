#ifndef _CRC_Client_H_
#define _CRC_Client_H_

#include "crc_base.h"
#include "crc_buffer.h"
#include "crc_net_work.h"

//客户端心跳检测死亡计时时间
#define CLIENT_HREAT_DEAD_TIME 120000
//在间隔指定时间后才允许发送
#define CLIENT_SEND_BUFF_TIME 200

enum ClientState
{
	clientState_create = 10,
	clientState_join,
	clientState_run,
	clientState_close,
};

//客户端数据类型
class CRCClient
{
//////////用于调试的成员变量
public:
	int id = -1;
	//所属serverid
	int serverId = -1;
	//测试接收发逻辑用
	//用于server检测接收到的消息ID是否连续
	int nRecvMsgID = 1;
	//测试接收发逻辑用
	//用于client检测接收到的消息ID是否连续
	int nSendMsgID = 1;
///////////////////////////////////
public:
	CRCClient(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);

	virtual ~CRCClient();

	inline SOCKET sockfd()
	{
		return _sockfd;
	}

	inline void resetDTHeart()
	{
		_dtHeart = 0;
	}

	inline void resetDTSend()
	{
		_dtSend = 0;
	}

	void destory();

	int  RecvData();

	virtual bool hasMsg();

	CRCDataHeader* front_msg();

	virtual void pop_front_msg();

	bool needWrite();

	//
	virtual void onSendComplete();

	//立即将发送缓冲区的数据发送给客户端
	int  SendDataReal();

	//缓冲区的控制根据业务需求的差异而调整
	//发送数据
	int  SendData(CRCDataHeader* header);

	int  SendData(const char* pData, int len);

	//心跳检测
	bool checkHeart(time_t dt);

	//定时发送消息检测
	bool checkSend(time_t dt);
	
	//设置客户端IP地址
	void setIP(char* ip);

	//返回客户端IP地址
	inline char* getIP()
	{
		return _ip;
	}

	inline ClientState state()
	{
		return _clientState;
	}

	inline void state(ClientState state)
	{
		_clientState = state;
	}

	void onClose();

	inline bool isClose()
	{
		return _clientState == clientState_close;
	}

#ifdef CRC_USE_IOCP
	IO_DATA_BASE* makeRecvIoData()
	{
		if (_isPostRecv)
			return nullptr;
		_isPostRecv = true;
		return _recvBuff.makeRecvIoData(_sockfd);
	}
	void recv4iocp(int nRecv)
	{
		if(!_isPostRecv)
			CRCLog_Error("recv4iocp _isPostRecv is false");
		_isPostRecv = false;
		_recvBuff.read4iocp(nRecv);
	}

	IO_DATA_BASE* makeSendIoData()
	{
		if (_isPostSend)
			return nullptr;
		_isPostSend = true;
		return _sendBuff.makeSendIoData(_sockfd);
	}

	void send2iocp(int nSend)
	{
		if (!_isPostSend)
			CRCLog_Error("send2iocp _isPostSend is false");
		_isPostSend = false;
		_sendBuff.write2iocp(nSend);
	}

	bool isPostIoAction()
	{
		return _isPostRecv || _isPostSend;
	}
#endif // CRC_USE_IOCP
protected:
	// socket fd_set  file desc set
	SOCKET _sockfd = INVALID_SOCKET;
	//发送缓冲区
	CRCBuffer _sendBuff;
	//第二缓冲区 接收消息缓冲区
	CRCBuffer _recvBuff;
	//心跳死亡计时
	time_t _dtHeart = 0;
	//上次发送消息数据的时间 
	time_t _dtSend = 0;
	//ip address
	char _ip[INET6_ADDRSTRLEN] = {};
#ifdef CRC_USE_IOCP
	bool _isPostRecv = false;
	bool _isPostSend = false;
#endif
	ClientState _clientState = clientState_create;
};

#endif // !_CRC_Client_H_