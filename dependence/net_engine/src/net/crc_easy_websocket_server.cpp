#include "crc_easy_websocket_server.h"

CRCClient* 
CRCEasyWebSocketServer::makeClientObj(SOCKET cSock)
{
    return new CRCClientSWebSocket(cSock, _nSendBuffSize, _nRecvBuffSize);
}

void 
CRCEasyWebSocketServer::OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header)
{
    CRCEasyTcpServer::OnNetMsg(pServer, pClient, header);
    CRCClientSWebSocket* pWSClient = dynamic_cast<CRCClientSWebSocket*>(pClient);
    if (!pWSClient)
        return;

    pWSClient->resetDTHeart();

    if (clientState_join == pWSClient->state())
    {	
        //握手
        if (!pWSClient->getRequestInfo())
            return;

        if (pWSClient->handshake())
            pWSClient->state(clientState_run);
        else
            pWSClient->onClose();
    }
    else if (clientState_run == pWSClient->state()) {
        CRCIO::WebSocketHeader& wsh = pWSClient->WebsocketHeader();
        if (wsh.opcode == CRCIO::opcode_PING)
        {
            pWSClient->pong();
        }
        else {
            //处理数据帧
            OnNetMsgWS(pServer, pWSClient);
        }
    }
}

void 
CRCEasyWebSocketServer::OnNetMsgWS(CRCWorkServer* pServer, CRCClientSWebSocket* pWSClient)
{

}