#ifndef _CRC_EASY_WEBSOCKET_SERVER_H_
#define _CRC_EASY_WEBSOCKET_SERVER_H_

#include "crc_tcp_server_mgr.h"
#include "crc_client_s_websocket.h"

class CRCEasyWebSocketServer : public CRCIO::TcpServerMgr
{
protected:
    virtual CRCClient* makeClientObj(SOCKET cSock);

    virtual void OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header);

public:
    virtual void OnNetMsgWS(CRCWorkServer* pServer, CRCClientSWebSocket* pWSClient);
};

#endif