#include "crc_net_server.h"

CRCClient* 
CRCNetServer::makeClientObj(SOCKET cSock)
{
    return new CRCNetClientS(cSock, _nSendBuffSize, _nRecvBuffSize);
}

void 
CRCNetServer::OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header)
{
    CRCEasyTcpServer::OnNetMsg(pServer, pClient, header);
    CRCNetClientS* pWSClient = dynamic_cast<CRCNetClientS*>(pClient);
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
CRCNetServer::OnNetLeave(CRCClient* pClient)
{
    CRCEasyTcpServer::OnNetLeave(pClient);

    CRCNetClientS* pWSClient = dynamic_cast<CRCNetClientS*>(pClient);
    if (!pWSClient)
        return;

    if (on_client_leave)
        on_client_leave(pWSClient);
}

void
CRCNetServer::OnNetMsgWS(CRCWorkServer* pServer, CRCNetClientS* pWSClient)
{
    CRCIO::WebSocketHeader& wsh = pWSClient->WebsocketHeader();
    if (wsh.opcode == CRCIO::opcode_PONG)
    {
        //CRCLog_Info("websocket client say: PONG");
        return;
    }
    auto dataStr = pWSClient->fetch_data();
    //CRCLog_Info("websocket client say: %s", dataStr);

    CRCJson json;
    if (!json.Parse(dataStr))
    {
        CRCLog_Error("json.Parse error : %s", json.GetErrMsg().c_str());
        return;
    }

    int msgId = 0;
    if (!json.Get("msgId", msgId))
    {
        CRCLog_Error("not found key<%s>.", "msgId");
        return;
    }

    time_t time = 0;
    if (!json.Get("time", time))
    {
        CRCLog_Error("not found key<%s>.", "time");
        return;
    }
    
    std::string cmd;
    if (!json.Get("cmd", cmd))
    {
        CRCLog_Error("not found key<%s>.", "cmd");
        return;
    }

    neb::CJsonObject data;
    if (!json.Get("data", data))
    {
        CRCLog_Error("not found key<%s>.", "data");
        return;
    }

    if (on_net_msg_do(pServer, pWSClient, cmd, json))
        return;

    on_other_msg(pServer, pWSClient, cmd, json);
}

void 
CRCNetServer::Init()
{
    const char* strIP = CRCConfig::Instance().getStr("strIP", "any");
    uint16_t nPort = CRCConfig::Instance().getInt("nPort", 4567);
    int nThread = 1;//CRCConfig::Instance().getInt("nThread", 1);

    if (strcmp(strIP, "any") == 0)
    {
        strIP = nullptr;
    }

    if (CRCConfig::Instance().hasKey("-ipv6"))
    {
        CRCLog_Info("-ipv6");
        this->InitSocket(AF_INET6);
    }
    else {
        CRCLog_Info("-ipv4");
        this->InitSocket();
    }

    this->Bind(strIP, nPort);
    this->Listen(SOMAXCONN);
    this->Start(nThread);
}

void 
CRCNetServer::reg_msg_call(std::string cmd, NetEventCall call)
{
    _map_msg_call[cmd] = call;
    CRCLog_Info("INetServer::reg_msg_call cmd<%s>.", cmd.c_str());
}

bool 
CRCNetServer::on_net_msg_do(CRCWorkServer* pServer, CRCNetClientS* pWSClient, std::string& cmd, neb::CJsonObject& msgJson)
{
    auto itr = _map_msg_call.find(cmd);
    if (itr != _map_msg_call.end())
    {
        itr->second(pServer, pWSClient, msgJson);
        return true;
    }
    CRCLog_Info("%s::INetServer::on_net_msg_do not found cmd<%s>.", pWSClient->link_name().c_str(), cmd.c_str());
    return false;
}