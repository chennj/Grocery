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
    
    //服务端响应
    bool is_resp = false;
    if (json.Get("is_resp", is_resp) && is_resp)
    {
        if (!pWSClient->is_ss_link())
        {
            CRCLog_Error("pWSClient->is_ss_link=false, is_resp=true.");
            return;
        }

        int clientId = 0;
        if (!json.Get("clientId", clientId))
        {
            CRCLog_Error("not found key<%s>.", "clientId");
            return;
        }

        auto client = dynamic_cast<CRCNetClientS*>( pServer->find_client(clientId));
        if (!client)
        {
            CRCLog_Error("CRCNetServer::OnNetMsgWS::pServer->find_client(%d) miss.", clientId);
            return;
        }
        if (SOCKET_ERROR == client->writeText(dataStr, wsh.len))
        {
            CRCLog_Error("CRCNetServer::OnNetMsgWS::sslink(%s)->clientId(%d) writeText SOCKET_ERROR.", pWSClient->link_name().c_str(), clientId);
        }

        return;
    }

    bool is_req = false;
    if (!json.Get("is_req", is_req))
    {
        CRCLog_Error("not found key<%s>.", "is_req");
        return;
    }

    //用户端请求
    //服务端请求
    if (is_req)
    {
        std::string cmd;
        if (!json.Get("cmd", cmd))
        {
            CRCLog_Error("not found key<%s>.", "cmd");
            return;
        }

        int clientId = (int)pWSClient->sockfd();
        json.Add("clientId", clientId);

        if (on_net_msg_do(pServer, pWSClient, cmd, json))
            return;

        on_other_msg(pServer, pWSClient, cmd, json);

        return;
    }

    CRCLog_Error("CRCNetServer::OnNetMsgWS:: is_req=false,  is_resp=false.");   
}

void 
CRCNetServer::Init()
{
    const char* strIP   = CRCConfig::Instance().getStr("strIP", "any");
    uint16_t nPort      = CRCConfig::Instance().getInt("nPort", 4567);
    int nThread         = CRCConfig::Instance().getInt("nThread", 1);

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
    CRCLog_Info("CRCNetServer::reg_msg_call cmd<%s>.", cmd.c_str());
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
    CRCLog_Info("%s::CRCNetServer::on_net_msg_do not found cmd<%s>.", pWSClient->link_name().c_str(), cmd.c_str());
    return false;
}