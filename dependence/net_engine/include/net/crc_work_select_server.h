#ifndef _CRC_WORK_SELECT_SERVER_H_
#define _CRC_WORK_SELECT_SERVER_H_

#include "crc_work_server.h"
#include "crc_fdset.h"

//网络消息接收处理服务类
class CRCWorkSelectServer : public CRCWorkServer
{
public:
	~CRCWorkSelectServer();

	virtual void setClientNum(int nSocketNum);

	bool DoNetEvents();

	void WriteData();

	void ReadData();

private:
	//伯克利套接字 BSD socket
	//描述符（socket） 集合
	CRCFDSet 	_fdRead;
	CRCFDSet 	_fdWrite;
	//备份客户socket fd_set
	CRCFDSet 	_fdRead_bak;
	//
	SOCKET 		_maxSock;
};

#endif // !_CRCSelectServer_HPP_
