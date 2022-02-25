#include "user_client.h"

void 
CRCUserClient::Init(int test_id)
{
    m_test_id   = test_id; 
    m_username  = "testaa" + std::to_string(test_id);
    m_password  = "testmm" + std::to_string(test_id);
    m_nickname  = "nname"  + std::to_string(test_id);

    //int s_size = Config::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
    //int r_size = Config::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

    m_csGate.connect("csGate",  m_s_csGateUrl.c_str(), SEND_BUFF_SZIE, RECV_BUFF_SZIE);

    m_csGate.reg_msg_call("onopen",     std::bind(&CRCUserClient::onopen_csGate,   this, std::placeholders::_1, std::placeholders::_2));
    m_csGate.reg_msg_call("onclose",    std::bind(&CRCUserClient::onclose_csGate,  this, std::placeholders::_1, std::placeholders::_2));

    m_csGate.reg_msg_call("sc_msg_logout", std::bind(&CRCUserClient::sc_msg_logout, this, std::placeholders::_1, std::placeholders::_2));

    m_csGate.reg_msg_call("sc_msg_group_join",  std::bind(&CRCUserClient::sc_msg_group_join,   this, std::placeholders::_1, std::placeholders::_2));
    m_csGate.reg_msg_call("sc_msg_group_exit",  std::bind(&CRCUserClient::sc_msg_group_exit,   this, std::placeholders::_1, std::placeholders::_2));
    m_csGate.reg_msg_call("sc_msg_group_say",   std::bind(&CRCUserClient::sc_msg_group_say,    this, std::placeholders::_1, std::placeholders::_2));
}

void 
CRCUserClient::Run()
{
    m_csGate.run(0);
    m_taskTimer.OnRun();
}

void 
CRCUserClient::Close()
{
    m_csGate.close();
}

void 
CRCUserClient::onopen_csGate(CRCNetClientC* client, CRCJson& msg)
{
    reg_client();
}

void 
CRCUserClient::onclose_csGate(CRCNetClientC* client, CRCJson& msg)
{

}

void 
CRCUserClient::reg_client()
{
    CRCJson json;
    json.Add("type", "Client");
    json.Add("cckey", "ccmm00@123456");
    if (!m_token.empty())
        json.Add("token", m_token);

    m_csGate.request("cs_reg_client", json, [this](CRCNetClientC* client, CRCJson& msg) {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { CRCLog_Error("cs_reg_client error <state=%d> msg: %s", state, msg("data").c_str()); return; }

        //CRCLog_Info(msg("data").c_str());

        if (!m_is_change_gate)
        {
            m_is_change_gate = true;
            login();
        }
        else {
            login_by_token();
        }
    });
}

void 
CRCUserClient::reg_user()
{
    CRCJson json;
    json.Add("username",    m_username);
    json.Add("password",    m_password);
    json.Add("nickname",    m_nickname);
    json.Add("sex",         m_test_id%2);

    m_csGate.request("cs_msg_register", json, [this](CRCNetClientC* client, CRCJson& msg) {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { CRCLog_Error("cs_msg_register error <state=%d> msg: %s", state, msg("data").c_str()); return; }


        if (!msg["data"].Get("userId", m_userId))
        {
            CRCLog_Error("not found key <userId>");
            return;
        }

        //CRCLog_Info("reg_user: userId=%lld", _userId);
        login();
    });
}

void 
CRCUserClient::login()
{
    CRCJson json;
    json.Add("username", m_username);
    json.Add("password", m_password);

    m_csGate.request("cs_msg_login", json, [this](CRCNetClientC* client, CRCJson& msg) 
    {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { 
            CRCLog_Error("cs_msg_login error <state=%d> msg: %s", state, msg("data").c_str()); 
            reg_user();
            return; 
        }

        
        if (!msg["data"].Get("userId", m_userId))
        {
            CRCLog_Error("not found key <userId>");
            return;
        }

        if (!msg["data"].Get("token", m_token))
        {
            CRCLog_Error("not found key <token>");
            return;
        }

        //CRCLog_Info("login: userId=%lld, token=%s", _userId, _token.c_str());
        change_run_gate();
    });
}

void 
CRCUserClient::login_by_token()
{
    CRCJson json;
    json.Add("token", m_token);

    m_csGate.request("cs_msg_login_by_token", json, [this](CRCNetClientC* client, CRCJson& msg) 
    {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { CRCLog_Error("cs_msg_login_by_token error <state=%d> msg: %s", state, msg("data").c_str()); return; }


        if (!msg["data"].Get("userId", m_userId))
        {
            CRCLog_Error("not found key <userId>");
            return;
        }

        if (!msg["data"].Get("token", m_token))
        {
            CRCLog_Error("not found key <token>");
            return;
        }

        //CRCLog_Info("login_by_token: userId=%lld, token=%s", _userId, _token.c_str());

        test_group();
    });
}

void 
CRCUserClient::sc_msg_logout(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("logout: %s", msg("data").c_str());
    m_userId = 0;
    m_token.clear();
}

void 
CRCUserClient::change_run_gate()
{
    CRCJson json;
    //json.Add("token", _token);
    //1.获取新业务网关的地址（LinkServer/LoginGate/RunGate）
    m_csGate.request("cs_get_run_gate", json,[this](CRCNetClientC* client, CRCJson& msg) 
    {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { CRCLog_Error("cs_get_run_gate error <state=%d> msg: %s", state, msg("data").c_str()); return; }

        std::string gate_url = msg("data");
        //CRCLog_Info("run_gate_url: %s", gate_url.c_str());

        //2.关闭原网络连接
        m_csGate.to_close();

        //3.连接新业务网关LinkServer
        m_csGate.connect("csGate", gate_url.c_str(), SEND_BUFF_SZIE, RECV_BUFF_SZIE);
    });
}

void 
CRCUserClient::test_group()
{
    //每个会话组多少人
    m_group_user_max = 10;
    //会话组的id和key
    m_group_id  = 20000 + (m_test_id / m_group_user_max);
    m_group_key = 10000 + (m_test_id / m_group_user_max);
    //会话组的创建者
    if (m_test_id%m_group_user_max == 0)
    {
        m_taskTimer.add_task_1s(0, 100, [this]() {
            group_create();
        });
    }
    else {//会话组的参与者
        m_taskTimer.add_task_1s(0, 2000, [this]() {
            group_join();
        });
    }

    m_nText = rand() % m_s_text_arr.size();

    int nd = (m_test_id % m_group_user_max);
    nd = 4000 + (nd * 100);

    m_taskTimer.add_task_1s(0, nd, [this]() 
    {
        m_taskTimer.add_task(0, m_s_nSendSleep, [this]() {
            group_say();
        });
    });
}

void 
CRCUserClient::group_create()
{
    CRCJson json;
    json.Add("group_id", m_group_id);
    json.Add("group_key",m_group_key);

    //CRCLog_Info("group_create1: test_id=%d, userId=%lld, group_id=%d, group_key=%d", _test_id, _userId, _group_id, _group_key);
    m_csGate.request("cs_msg_group_create", json, [this](CRCNetClientC* client, CRCJson& msg) 
    {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { CRCLog_Error("cs_msg_group_create error <state=%d> msg: %s", state, msg("data").c_str()); return; }
        
        int g_id = 0;
        if (!msg["data"].Get("group_id", g_id))
        {
            CRCLog_Error("not found key <group_id>");
            return;
        }

        int g_key = 0;
        if (!msg["data"].Get("group_key", g_key))
        {
            CRCLog_Error("not found key <group_key>");
            return;
        }

        //CRCLog_Info("group_create2: test_id=%d, userId=%lld, group_id=%d, group_key=%d", _test_id, _userId, g_id, g_key);
    });
}

void 
CRCUserClient::group_join()
{
    CRCJson json;
    json.Add("group_id",    m_group_id);
    json.Add("group_key",   m_group_key);

    //CRCLog_Info("group_join1: test_id=%d, userId=%lld, group_id=%d, group_key=%d", _test_id, _userId, _group_id, _group_key);
    m_csGate.request("cs_msg_group_join", json, [this](CRCNetClientC* client, CRCJson& msg) 
    {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { CRCLog_Error("cs_msg_group_join error <state=%d> msg: %s", state, msg("data").c_str()); return; }

        int g_id = 0;
        if (!msg["data"].Get("group_id", g_id))
        {
            CRCLog_Error("not found key <group_id>");
            return;
        }

        int g_key = 0;
        if (!msg["data"].Get("group_key", g_key))
        {
            CRCLog_Error("not found key <group_key>");
            return;
        }

        //CRCLog_Info("group_join2: test_id=%d, userId=%lld, group_id=%d, group_key=%d", _test_id, _userId, g_id, g_key);
    });
}

void 
CRCUserClient::group_say()
{
    CRCJson json;
    json.Add("group_id",  m_group_id);
    json.Add("group_key", m_group_key);

    
    if (++m_nText >= m_s_text_arr.size())
        m_nText = 0;
    m_nText = 23;
    json.Add("say", m_s_text_arr.at(m_nText));

    m_csGate.request("cs_msg_group_say", json, [this](CRCNetClientC* client, CRCJson& msg) {
        int state = STATE_CODE_OK;
        if (!msg.Get("state", state)) { CRCLog_Error("not found key <state>"); return; }
        if (state != STATE_CODE_OK) { CRCLog_Error("cs_msg_group_say error <state=%d> msg: %s", state, msg("data").c_str()); return; }

    });
}

void 
CRCUserClient::group_exit()
{

}

void 
CRCUserClient::sc_msg_group_join(CRCNetClientC* client, CRCJson& msg)
{
    int g_id = 0;
    if (!msg["data"].Get("group_id", g_id))
    {
        CRCLog_Error("not found key <group_id>");
        return;
    }

    int clientId = 0;
    if (!msg["data"].Get("clientId", clientId))
    {
        CRCLog_Error("not found key <clientId>");
        return;
    }

    //CRCLog_Info("test_id=%d, userId=%lld >> sc_msg_group_join: group_id=%d, clientId=%d", _test_id, _userId, g_id, clientId);
}

void 
CRCUserClient::sc_msg_group_exit(CRCNetClientC* client, CRCJson& msg)
{
    int g_id = 0;
    if (!msg["data"].Get("group_id", g_id))
    {
        CRCLog_Error("not found key <group_id>");
        return;
    }

    int clientId = 0;
    if (!msg["data"].Get("clientId", clientId))
    {
        CRCLog_Error("not found key <clientId>");
        return;
    }

    //CRCLog_Info("test_id=%d, userId=%lld >> sc_msg_group_exit: group_id=%d, clientId=%d", _test_id, _userId, g_id, clientId);
}

void 
CRCUserClient::sc_msg_group_say(CRCNetClientC* client, CRCJson& msg)
{
    int g_id = 0;
    if (!msg["data"].Get("group_id", g_id))
    {
        CRCLog_Error("not found key <group_id>");
        return;
    }

    int clientId = 0;
    if (!msg["data"].Get("clientId", clientId))
    {
        CRCLog_Error("not found key <clientId>");
        return;
    }

    std::string say;
    if (!msg["data"].Get("say", say))
    {
        CRCLog_Error("not found key <say>");
        return;
    }

    //CRCLog_Info("test_id=%d, userId=%lld >> sc_msg_group_say: group_id=%d, clientId=%d, say=%s", _test_id, _userId, g_id, clientId, say.c_str());
}