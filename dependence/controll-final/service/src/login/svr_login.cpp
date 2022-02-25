#include <regex>

#include "svr_login.h"

void 
LoginServer::Init()
{
    m_dbuser.init();

    m_csGate.set_groupId("0001");

    m_csGate.connect("csCtrl","ws://192.168.137.129:4567", 1024 * 1024 * 10, 1024 * 1024 * 10);

    m_csGate.reg_msg_call("onopen", std::bind(&LoginServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    m_csGate.reg_msg_call("cs_msg_login",           std::bind(&LoginServer::cs_msg_login,           this, std::placeholders::_1, std::placeholders::_2));
    m_csGate.reg_msg_call("cs_msg_register",        std::bind(&LoginServer::cs_msg_register,        this, std::placeholders::_1, std::placeholders::_2));
    m_csGate.reg_msg_call("cs_msg_change_pw",       std::bind(&LoginServer::cs_msg_change_pw,       this, std::placeholders::_1, std::placeholders::_2));
    m_csGate.reg_msg_call("cs_msg_login_by_token",  std::bind(&LoginServer::cs_msg_login_by_token,  this, std::placeholders::_1, std::placeholders::_2));
    
    m_csGate.reg_msg_call("ss_msg_client_exit",     std::bind(&LoginServer::ss_msg_client_exit,     this, std::placeholders::_1, std::placeholders::_2));
    m_csGate.reg_msg_call("ss_msg_user_exit",       std::bind(&LoginServer::ss_msg_user_exit,       this, std::placeholders::_1, std::placeholders::_2));
}

void 
LoginServer::Run()
{
    m_csGate.run(1);
}

void 
LoginServer::Close()
{
    m_csGate.close();
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
    json["apis"].Add("cs_msg_login_by_token");
    json["apis"].Add("ss_msg_client_exit");
    json["apis"].Add("ss_msg_user_exit");

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

void 
LoginServer::cs_msg_login_by_token(CRCNetClientC* client, CRCJson& msg)
{
    int clientId = 0;
    if (!msg.Get("clientId", clientId))
    {
        CRCLog_Error("not found key<clientId>.");
        return;
    }

    //当前请求字段获取与验证
    std::string token;
    {
        if (!msg["data"].Get("token", token))
        {
            client->resp_error(msg, "not found key <token>.");
            return;
        }

        if (token.empty())
        {
            client->resp_error(msg, "<token> can not be empty!");
            return;
        }
        //正则表达式
        std::regex reg1("^[0-9a-zA-Z]{6,128}$");
        if (!regex_match(token, reg1))
        {
            client->resp_error(msg, "<token> format is incorrect!");
            return;
        }
    }
    //
    //CELLLog_Info("LoginServer::cs_msg_login_by_token: token=%s", token.c_str());
    //判断是否登录过
    //在线验证
    /*
    auto user = m_userManager.get_by_token(token);
    if (!user)
    {
        client->resp_error(msg, "Invalid token!");
        return;
    }

    int64_t userId = user->userId;
    {
        if (user->is_online()) {
            //通知当前已登录用户有人在其它地方登录
            client->push(user->clientId, "sc_msg_logout", "Someone is trying to login this account!");

            //通知网关用户登出
            CRCJson ret;
            ret.Add("userId", user->userId);
            ret.Add("token", user->token);
            ret.Add("clientId", user->clientId);
            int linkId = ClientId::get_link_id(user->clientId);
            client->push(linkId, "ss_msg_user_logout", ret);
        }

        //将已登录用户移除
        _userManager.remove(user);
    }
    //签发令牌 生成登录令牌
    token = make_token(userId, clientId);
    //记录令牌 关联用户数据
    if (!_userManager.add(token, userId, clientId))
    {
        client->resp_error(msg, "userManager add failed!");
        return;
    }
    //通知网关用户登录
    CRCJson ret;
    ret.Add("userId", userId);
    ret.Add("token", token);
    ret.Add("clientId", clientId);
    int linkId = ClientId::get_link_id(clientId);
    client->push(linkId, "ss_msg_user_login", ret);
   
    //返回登录结果
    CRCJson json;
    json.Add("userId", userId);
    json.Add("token", token);
    client->response(msg, json);
    */

    CRCJson json;
    json.Add("userId", "test");
    json.Add("token", token);
    client->response(msg, json);
}

void 
LoginServer::ss_msg_client_exit(CRCNetClientC* client, CRCJson& msg)
{
    int clientId = 0;
    if (!msg["data"].Get("clientId", clientId))
    {
        client->resp_error(msg, "not found key<clientId>.");
        return;
    }
    //CELLLog_Info("ss_msg_client_exit: clientId<%d>.", clientId);

    //判断是否登录过
    //在线验证
    /*
    auto user = _userManager.get_by_clientId(clientId);
    if (user)
    {
        //设置user为离线状态
        user->offline();
        //移除用户记录
        //_userManager.remove(user);
    }*/
}

void 
LoginServer::ss_msg_user_exit(CRCNetClientC* client, CRCJson& msg)
{
    int clientId = 0;
    if (!msg["data"].Get("clientId", clientId))
    {
        client->resp_error(msg, "not found key<clientId>.");
        return;
    }
    int64_t userId = 0;
    if (!msg["data"].Get("userId", userId))
    {
        client->resp_error(msg, "not found key<userId>.");
        return;
    }
    //CRCLog_Info("ss_msg_user_exit: clientId<%d> userId<%lld>.", clientId, userId);
}