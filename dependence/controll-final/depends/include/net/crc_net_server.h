/**
 * 
 * author:  chenningjiang
 * desc:    websocket协议的通用接收服务端
 * 
 * */
#ifndef _CRC_NET_SERVER_H_
#define _CRC_NET_SERVER_H_

#include "crc_easy_websocket_server.h"
#include "CJsonObject.hpp"
#include "crc_net_client_s.h"

class CRCNetServer : public  CRCEasyWebSocketServer
{
private:
	typedef std::function<void(CRCWorkServer*, CRCNetClientS*, neb::CJsonObject&)> NetEventCall;
	std::map<std::string, NetEventCall> m_map_msg_call;

public:
	std::function<void(CRCWorkServer*, CRCNetClientS*, std::string&, neb::CJsonObject&)> on_other_msg 		= nullptr;
	std::function<void(CRCWorkServer*, CRCNetClientS*, std::string&, neb::CJsonObject&)> on_broadcast_msg 	= nullptr;

	std::function<void(CRCNetClientS*)> on_client_leave = nullptr;
	std::function<void(CRCWorkServer*)> on_net_run 		= nullptr;

private:
	virtual CRCClient* makeClientObj(SOCKET cSock) override;

	virtual void OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header) override;

	virtual void OnNetLeave(CRCClient* pClient) override;

	virtual void OnNetRun(CRCWorkServer* pServer);

public:
	virtual void OnNetMsgWS(CRCWorkServer* pServer, CRCNetClientS* pWSClient);

	void Init();

	void reg_msg_call(std::string cmd, NetEventCall call);

	bool on_net_msg_do(CRCWorkServer* pServer, CRCNetClientS* pWSClient, std::string& cmd, neb::CJsonObject& msgJson);
};

#endif //!_CRC_NET_SERVER_H_