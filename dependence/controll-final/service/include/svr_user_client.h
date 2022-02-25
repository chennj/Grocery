#ifndef _USER_CLIENT_H_
#define _USER_CLIENT_H_

#include "crc_net_client_c.h"
#include "crc_task_timer.h"

class CRCUserClient
{
public:
    static std::string              m_s_csGateUrl;
    static std::vector<std::string> m_s_text_arr;
    static int                      m_s_nSendSleep;
private:
    CRCTaskTimer    m_taskTimer;
    CRCNetClientC   m_csGate;
    int64_t         m_userId = 0;
    std::string     m_token;
    bool            m_is_change_gate = false;

    int             m_test_id = 0;
    std::string     m_username;
    std::string     m_password;
    std::string     m_nickname;

    //每个会话组多少人
    int m_group_user_max = 3;
    //会话组的id和key
    int m_group_id = 0;
    int m_group_key = 0;
    int m_nText = 0;
public:
    void Init(int test_id);

    void Run();

    void Close();

private:
    void onopen_csGate(CRCNetClientC* client, CRCJson& msg);

    void onclose_csGate(CRCNetClientC* client, CRCJson& msg);

    void reg_client();

    void reg_user();

    void login();

    void login_by_token();

    void sc_msg_logout(CRCNetClientC* client, CRCJson& msg);

    void change_run_gate();

    void test_group();

    void group_create();

    void group_join();

    void group_say();

    void group_exit();

    void sc_msg_group_join(CRCNetClientC* client, CRCJson& msg);

    void sc_msg_group_exit(CRCNetClientC* client, CRCJson& msg);

    void sc_msg_group_say(CRCNetClientC* client, CRCJson& msg);
};
#endif