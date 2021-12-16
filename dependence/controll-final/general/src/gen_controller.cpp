#include "gen_controller.h"

void 
GenController::Init()
{
    _netserver.Init();
    _netserver.on_other_msg     = std::bind(&GenController::on_other_msg, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    _netserver.on_client_leave  = std::bind(&GenController::on_client_leave, this, std::placeholders::_1);
    _netserver.reg_msg_call("cs_msg_heart", std::bind(&GenController::cs_msg_heart, this,std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    _netserver.reg_msg_call("ss_reg_api", std::bind(&GenController::ss_reg_api, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void 
GenController::Close()
{
    _netserver.Close();
}

void 
GenController::cs_msg_heart(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg)
{
    CRCLog_Info("GenController::cs_msg_heart");

    CRCJson ret;
    ret.Add("data", "wo ye bu ji dao.");
    client->response(msg, ret);

    //client->respone(msg, "wo ye bu ji dao.");
}

void 
GenController::ss_reg_api(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg)
{
    auto sskey = msg["data"]("sskey");
    auto sskey_local = CRCConfig::Instance().getStr("sskey", "ssmm00@123456");
    if (sskey != sskey_local)
    {
        CRCJson ret;
        ret.Add("state", 0);
        ret.Add("msg", "sskey error.");
        client->response(msg, ret);
        return;
    }
    auto type = msg["data"]("type");
    auto name = msg["data"]("name");

    client->link_type(type);
    client->link_name(name);
    client->is_ss_link(true);

    auto apis = msg["data"]["apis"];

    if (!apis.IsArray())
    {
        CRCJson ret;
        ret.Add("state", 0);
        ret.Add("msg", "not found apis.");
        client->response(msg, ret);
        return;
    }
    int size = apis.GetArraySize();
    for (size_t i = 0; i < size; i++)
    {
        CRCLog_Info("ss_reg_api: %s >> %s", name.c_str(), apis(i).c_str());
        _transfer.add(apis(i), client);
    }
}

void 
GenController::on_other_msg(CRCWorkServer* server, CRCNetClientS* client, std::string& cmd, CRCJson& msg)
{
    auto str = msg.ToString();
    if (!_transfer.on_net_msg_do(cmd, str))
    {
        CRCLog_Info("on_other_msg: transfer not found cmd<%s>.", cmd.c_str());
    }
}

void 
GenController::on_client_leave(CRCNetClientS* client)
{
    if(client->is_ss_link())
        _transfer.del(client);
}