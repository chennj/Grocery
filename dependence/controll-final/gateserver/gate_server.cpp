#include "gate_server.h"

void 
GateServer::Init()
{
    m_netserver.Init();

    m_netserver.on_other_msg     = std::bind(&GateServer::on_other_msg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    m_netserver.on_broadcast_msg = std::bind(&GateServer::on_broadcast_msg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    m_netserver.on_client_leave  = std::bind(&GateServer::on_client_leave, this, std::placeholders::_1);
    
    m_netserver.reg_msg_call("cs_msg_heart",    std::bind(&GateServer::cs_msg_heart, this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_netserver.reg_msg_call("ss_reg_api",      std::bind(&GateServer::ss_reg_api, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void 
GateServer::Close()
{
    m_netserver.Close();
}

void 
GateServer::cs_msg_heart(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg)
{
    if (client->is_cc_link() || client->is_ss_link())
    {
        //CRCLog_Info("GateServer::cs_msg_heart");

        //CRCJson ret;
        //ret.Add("data", "wo ye bu ji dao.");
        //client->response(msg, ret);

        //client->respone(msg, "wo ye bu ji dao.");
    }
}

void 
GateServer::ss_reg_api(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg)
{
    auto gpid   = msg("groupId");   
    auto sskey  = msg["data"]("sskey");
    auto sskey_local = CRCConfig::Instance().getStr("sskey", "ssmm00@123456");
    if (sskey != sskey_local)
    {
        CRCJson ret;
        ret.Add("state", 0);
        ret.Add("msg", "sskey error.");
        client->resp_error(msg, ret);
        return;
    }
    if (gpid.empty()){
        gpid = "0000";
    }

    auto type = msg["data"]("type");
    auto name = msg["data"]("name");

    client->link_type(type);
    client->link_name(name);
    client->link_group(gpid);
    client->is_ss_link(true);

    auto apis = msg["data"]["apis"];

    if (!apis.IsArray())
    {
        CRCJson ret;
        ret.Add("state", 0);
        ret.Add("msg", "not found apis.");
        client->resp_error(msg, ret);
        return;
    }

    int size = apis.GetArraySize();
    for (size_t i = 0; i < size; i++)
    {
        std::string cmd = gpid + ":" + apis(i);
        m_transfer.add(cmd, client);
        CRCLog_Info("ss_reg_api: %s >> %s", name.c_str(), cmd.c_str());
    }

    CRCJson json;
	json.Add("ClientId", client->clientId());
    client->response(msg, json);
}

void 
GateServer::on_other_msg(CRCWorkServer* server, CRCNetClientS* client, std::string& cmd, CRCJson& msg)
{
    auto str = msg.ToString();
    int  ret = m_transfer.on_net_msg_do(cmd, str);

    if (STATE_CODE_UNDEFINE_CMD == ret)
    {
        CRCLog_Info("on_other_msg: transfer not found cmd<%s>.", cmd.c_str());
        client->response(msg, "undefine cmd!", STATE_CODE_UNDEFINE_CMD);
    }
    else if (STATE_CODE_SERVER_BUSY == ret)
    {
        CRCLog_Info("on_other_msg: server busy! cmd<%s>.", cmd.c_str());
        client->response(msg, "server busy!", STATE_CODE_SERVER_BUSY);
    }
    else if (STATE_CODE_SERVER_OFF == ret)
    {
        CRCLog_Info("on_other_msg: server offline! cmd<%s>.", cmd.c_str());
        client->response(msg, "server offline!", STATE_CODE_SERVER_OFF);
    }
}

void 
GateServer::on_broadcast_msg(CRCWorkServer* server, CRCNetClientS* client, std::string& cmd, CRCJson& msg)
{
    auto str = msg.ToString();
	m_transfer.on_broadcast_do(cmd, str);
}

void 
GateServer::on_client_leave(CRCNetClientS* client)
{
    if(client->is_ss_link()){
        m_transfer.del(client);
    }

    if (client->is_login())
    {
        CRCJson msg;
        msg.Add("clientId", client->clientId());
        msg.Add("userId",   client->userId());
        broadcast("ss_msg_user_exit", msg);
    }

    {
        CRCJson msg;
        msg.Add("clientId", client->clientId());
        broadcast("ss_msg_client_exit", msg);
    }
}