#ifndef _VIRTUAL_SERVER_H_
#define _VIRTUAL_SERVER_H_

#include "crc_easy_select_server.h"
#include "crc_msg_stream.h"
#include "crc_config.h"

class VirtualServer : public CRCEasySelectServer
{
public:
	VirtualServer();

	virtual void OnNetJoin(CRCClient* pClient) override;

	virtual void OnNetLeave(CRCClient* pClient) override;

	virtual void OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header) override;
private:
	//自定义标志 收到消息后将返回应答消息
	bool _bSendBack;
	//自定义标志 是否提示：发送缓冲区已写满
	bool _bSendFull;
	//是否检查接收到的消息ID是否连续
	bool _bCheckMsgID;
};

#endif  //!_SVR_MACHINE_SERVER_H_