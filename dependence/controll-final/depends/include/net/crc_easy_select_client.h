#ifndef _CRC_EASY_SELECT_CLIENT_H_
#define _CRC_EASY_SELECT_CLIENT_H_

#include "crc_easy_tcp_client.h"
#include "crc_fdset.h"

class CRCEasySelectClient : public CRCEasyTcpClient
{
public:
	CRCEasySelectClient();
    
	//处理网络消息
	bool OnRun(int microseconds = 1);
	
protected:
	CRCFDSet _fdRead;
	CRCFDSet _fdWrite;
};

#endif //!_CRC_EASY_SELECT_CLIENT_H_