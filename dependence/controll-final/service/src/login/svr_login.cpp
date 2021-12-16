#include "svr_login.h"

void 
LoginServer::Init()
{
    _csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    _csCtrl.reg_msg_call("onopen", std::bind(&LoginServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    _csCtrl.reg_msg_call("cs_msg_heart", std::bind(&LoginServer::cs_msg_heart, this,std::placeholders::_1, std::placeholders::_2));
    _csCtrl.reg_msg_call("cs_msg_login", std::bind(&LoginServer::cs_msg_login, this, std::placeholders::_1, std::placeholders::_2));
}

void 
LoginServer::Run()
{
    _csCtrl.run(1);
}

void 
LoginServer::Close()
{
    _csCtrl.close();
}

void 
LoginServer::onopen_csCtrl(CRCNetClientC* client, CRCJson& msg)
{
    CRCJson json;
    json.Add("type", "LoginServer");
    json.Add("name", "LoginServer001");
    json.Add("sskey", "ssmm00@123456");
    json.AddEmptySubArray("apis");
    json["apis"].Add("cs_msg_login");
    json["apis"].Add("cs_msg_register");
    json["apis"].Add("cs_msg_change_pw");

    client->request("ss_reg_api", json);
}

void 
LoginServer::cs_msg_heart(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("LoginServer::cs_msg_heart");

    CRCJson ret;
    ret.Add("data", "wo ye bu ji dao.");
    client->response(msg, ret);

    //client->respone(msg, "wo ye bu ji dao.");
}

void 
LoginServer::cs_msg_login(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("LoginServer::cs_msg_login");
}