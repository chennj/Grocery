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
CRCNetServer::OnNetRun(CRCWorkServer* pServer)
{
    if (on_net_run){
        on_net_run(pServer);
    }
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
    auto pStr = pWSClient->fetch_data();
    std::string dataStr(pStr, wsh.len);
    //CRCLog_Info("websocket client say: %s", dataStr);

    CRCJson json;
    if (!json.Parse(dataStr))
    {
        CRCLog_Error("json.Parse error : %s", json.GetErrMsg().c_str());
        return;
    }

    /*
    int msgId = 0;
    if (!json.Get("msgId", msgId))
    {
        CRCLog_Error("not found key<msgId>.");
        return;
    }

    time_t time = 0;
    if (!json.Get("time", time))
    {
        CRCLog_Error("not found key<time>.");
        return;
    }

    std::string groupId;
    if (!json.Get("groupId", groupId))
    {
        CRCLog_Error("not found key<groupId>.");
        return;
    }*/

    int msg_type = 0;
    if (!json.Get("type", msg_type))
    {
        CRCLog_Error("not found key<type>.");
        return;
    }

    
    //服务端响应
    //服务端推送
    if (MSG_TYPE_RESP == msg_type ||
        MSG_TYPE_PUSH == msg_type)
    {
        if (!pWSClient->is_ss_link())
        {
            CRCLog_Error("pWSClient->is_ss_link=false, msg_type = %d.", msg_type);
            return;
        }

        int clientId = 0;
        if (!json.Get("clientId", clientId))
        {
            CRCLog_Error("not found key<clientId>.");
            return;
        }
        //优先取得linkServer的Id进行转发消息
        clientId = ClientId::get_link_id(clientId);
        auto client = dynamic_cast<CRCNetClientS*>( find_client(clientId));
        if (!client)
        {
            CRCLog_Error("CRCNetServer::OnNetMsgWS::find_client(%d) miss.", clientId);
            return;
        }
        if (SOCKET_ERROR == client->writeText(dataStr.c_str(), dataStr.length()))
        {
            CRCLog_Error("CRCNetServer::OnNetMsgWS::sslink(%s)->clientId(%d) writeText SOCKET_ERROR.", pWSClient->link_name().c_str(), clientId);
        }

        return;
    }

    //服务端批量推送
    if (MSG_TYPE_PUSH_S == msg_type)
    {
        if (!pWSClient->is_ss_link())
        {
            CRCLog_Error("pWSClient->is_ss_link=false, msg_type = %d.", msg_type);
            return;
        }
        //获得目标用户id
        auto clients = json["clients"];
        if (!clients.IsArray())
        {
            CRCLog_Error("not found key<clients>.");
            return;
        }
        //移除原始消息目标用户数组
        //json.Delete("clients");
        json.Replace("type", MSG_TYPE_PUSH);
        //遍历目标开始推送
        int size = clients.GetArraySize();
        int clientId = 0;
        //json.Add("clientId", clientId);
        //for (size_t i = 0; i < size; i++)
        //{
        //	if (!clients.Get(i, clientId))
        //		continue;
        //	//每条推送消息的目标不同
        //	json.Replace("clientId", clientId);
        //	//
        //	clientId = ClientId::get_link_id(clientId);
        //	auto client = dynamic_cast<INetClientS*>(find_client(clientId));
        //	if (!client)
        //	{
        //		CELLLog_Error("INetServer::OnNetMsgWS::find_client(%d) miss.", clientId);
        //		return;
        //	}
        //	client->transfer(json);
        //}

        //分类<linkid,clients>
        std::map<CRCNetClientS*, CRCJson> temp;
        for (size_t i = 0; i < size; i++)
        {
            if (!clients.Get(i, clientId))
                continue;

            auto link_id = ClientId::get_link_id(clientId);
            auto link_gate = dynamic_cast<CRCNetClientS*>(find_client(link_id));
            if (!link_gate)
            {
                CRCLog_Error("INetServer::OnNetMsgWS::find_link_gate(%d) miss.", link_id);
                continue;
            }
            //相同的linkServer上的client存储在一起
            auto itr = temp.find(link_gate);
            if (itr == temp.end())
            {
                neb::CJsonObject clients;
                clients.Add(clientId);
                temp[link_gate] = clients;
            }
            else {
                itr->second.Add(clientId);
            }
        }
        //批量推送消息给linkServer
        for (auto& itr : temp)
        {
            json.Delete("clients");
            json.Add("clients", itr.second);
            itr.first->transfer(json);
        }
        return;
    }

    //用户端请求
    //服务端请求
    if (MSG_TYPE_REQ == msg_type)
    {
        std::string groupId;
        if (!json.Get("groupId", groupId))
        {
            CRCLog_Error("not found key<groupId>.");
            return;
        }

        std::string cmd;
        if (!json.Get("cmd", cmd))
        {
            CRCLog_Error("not found key<%s>.", "cmd");
            return;
        }
        //
        int clientId = 0;
        if (!json.Get("clientId", clientId))
        {
            json.Add("clientId", pWSClient->clientId());
        }
        else {
            clientId = ClientId::set_link_id(pWSClient->clientId(), clientId);
            json.Replace("clientId", clientId);
        }

        //is_cc 标识是否为LinkGate转发的客户端请求
        bool is_cc = false;
        json.Get("is_cc", is_cc);
        if (!is_cc)
        //非LinkGate转发的请求，才做以下处理
        {
            if (pWSClient->is_login())
            {
                json.Add("userId", pWSClient->userId());
            }

            if (pWSClient->is_ss_link())
            {
                json.Add("is_ss_link", true, true);
            }
        }
        else
        //如果是客户端直接连接
        {
            pWSClient->is_cc_link(true);          
        }

        //网关本地支持的指令
        if (on_net_msg_do(pServer, pWSClient, cmd, json)){
            return;
        }

        //服务提供的服务，需要分组
        if (pWSClient->is_cc_link() || pWSClient->is_ss_link())
        {
            std::string gpidcmd = groupId + ":" + cmd;
            on_other_msg(pServer, pWSClient, gpidcmd, json);
            return;
        }

        //如果是通过控制器执行业务
        std::string proxy_ctl;
        if (!json.Get("proxy_ctl", proxy_ctl))
        {
            CRCLog_Warring("not found key<proxy_ctl>.");
        } else {
            if (proxy_ctl.compare("nlctroller") == 0){
                std::string gpidcmd = "0000:" + cmd;
                on_other_msg(pServer, pWSClient, gpidcmd, json);
                return;
            }
            CRCLog_Warring("not found controller<s%>.",proxy_ctl.c_str());
        }

        return;
    }

    //只有服务可以发出广播
    if (MSG_TYPE_BROADCAST == msg_type)
    {
        if (!pWSClient->is_ss_link())
        {
            CRCLog_Error("pWSClient->is_ss_link=false, msg_type = %d.", msg_type);
            return;
        }

        std::string cmd;
        if (!json.Get("cmd", cmd))
        {
            CRCLog_Error("not found key<%s>.", "cmd");
            return;
        }

        json.Add("clientId", pWSClient->clientId());

        if (pWSClient->is_login())
        {
            json.Add("userId", pWSClient->userId());
        }

        if (pWSClient->is_ss_link())
        {
            json.Add("is_ss_link", true, true);
        }

        on_broadcast_msg(pServer, pWSClient, cmd, json);

        return;
    }

    CRCLog_Error("CRCNetServer::OnNetMsgWS:: unknow msg type <%d>.", msg_type);

    return; 
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
    m_map_msg_call[cmd] = call;
    CRCLog_Info("CRCNetServer::reg_msg_call cmd<%s>.", cmd.c_str());
}

bool 
CRCNetServer::on_net_msg_do(CRCWorkServer* pServer, CRCNetClientS* pWSClient, std::string& cmd, CRCJson& msgJson)
{
    auto itr = m_map_msg_call.find(cmd);
    if (itr != m_map_msg_call.end())
    {
        itr->second(pServer, pWSClient, msgJson);
        return true;
    }
    CRCLog_Info("%s::CRCNetServer::on_net_msg_do not found cmd<%s>.", pWSClient->link_name().c_str(), cmd.c_str());
    return false;
}