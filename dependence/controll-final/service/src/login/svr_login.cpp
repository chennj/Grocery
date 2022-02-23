#include <regex>

#include "svr_login.h"

void 
LoginServer::Init()
{
    m_csCtrl.set_groupId("0001");

    m_csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    m_csCtrl.reg_msg_call("onopen", std::bind(&LoginServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    m_csCtrl.reg_msg_call("cs_msg_login",        std::bind(&LoginServer::cs_msg_login,       this, std::placeholders::_1, std::placeholders::_2));
    m_csCtrl.reg_msg_call("cs_msg_register",     std::bind(&LoginServer::cs_msg_register,    this, std::placeholders::_1, std::placeholders::_2));
    m_csCtrl.reg_msg_call("cs_msg_change_pw",    std::bind(&LoginServer::cs_msg_change_pw,   this, std::placeholders::_1, std::placeholders::_2));
}

void 
LoginServer::Run()
{
    m_csCtrl.run(1);
}

void 
LoginServer::Close()
{
    m_csCtrl.close();
}

void 
LoginServer::onopen_csCtrl(CRCNetClientC* client, CRCJson& msg)
{
    CRCJson json;
    json.Add("type",    "LoginServer");
    json.Add("name",    "LoginServer001");
    json.Add("sskey",   "ssmm00@123456");
    json.AddEmptySubArray("apis");
    json["apis"].Add("cs_msg_login");
    json["apis"].Add("cs_msg_register");
    json["apis"].Add("cs_msg_change_pw");

    client->request("ss_reg_api", json, [](CRCNetClientC* client, CRCJson& msg) {
        CRCJson cj; 
        msg.Get("data", cj);
        CRCLog_Info("LoginServer::ss_reg_api return: %s", cj.ToString().c_str());
    });
}

void 
LoginServer::cs_msg_login(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("LoginServer::cs_msg_login");

    CRCJson ret;
    ret.Add("data", "login successs.");
    client->response(msg, ret);
}

void 
LoginServer::cs_msg_register(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("LoginServer::cs_msg_register");

    //当前请求字段获取与验证
    std::string username;
    std::string password;
    std::string nickname;
    int sex = -1;
    {
        if (!msg["data"].Get("username", username))
        {
            client->resp_error(msg, "not found key <username>.");
            return;
        }

        if (username.empty())
        {
            client->resp_error(msg, "<username> can not be empty!");
            return;
        }
        //正则表达式
        std::regex reg1("^[0-9a-zA-Z]{6,16}$");
        if (!regex_match(username, reg1))
        {
            client->resp_error(msg, "<username> format is incorrect!");
            return;
        }

        if (!msg["data"].Get("password", password))
        {
            client->resp_error(msg, "not found key<password>.");
            return;
        }

        if (password.empty())
        {
            client->resp_error(msg, "<password> can not be empty!");
            return;
        }

        //正则表达式
        if (!regex_match(password, reg1))
        {
            client->resp_error(msg, "<password> format is incorrect!");
            return;
        }

        if (!msg["data"].Get("nickname", nickname))
        {
            client->resp_error(msg, "not found key<nickname>.");
            return;
        }

        if (nickname.empty())
        {
            client->resp_error(msg, "<nickname> can not be empty!");
            return;
        }

        if (nickname.length() <3 || nickname.length() > 16)
        {
            client->resp_error(msg, "<nickname> format is incorrect!");
            return;
        }

        if (!msg["data"].Get("sex", sex))
        {
            client->resp_error(msg, "not found key<sex>.");
            return;
        }

        if (sex !=0 && sex != 1)
        {
            client->resp_error(msg, "<sex> is only 0 or 1!");
            return;
        }
    }

    CRCJson ret;
    ret.Add("data", "register successs.");
    client->response(msg, ret);
}

void 
LoginServer::cs_msg_change_pw(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("LoginServer::cs_msg_change_pw");

    CRCJson ret;
    ret.Add("data", "change password successs.");
    client->response(msg, ret);
}