/**
 * 
 * author:  chenningjiang
 * desc:    登录服务器（实际是一个连接客户端）
 * 
 * */
#ifndef _SVR_LOGIN_H_
#define _SVR_LOGIN_H_

#include "crc_net_client_c.h"
#include "crc_db_user.h"

class LoginServer
{
private:
    CRCNetClientC   m_csCtrl;
    CRCDBUser       m_dbuser;
public:
    void Init();

    void Run();

    void Close();

private:
    void onopen_csCtrl          (CRCNetClientC* client, CRCJson& msg);

    void cs_msg_login           (CRCNetClientC* client, CRCJson& msg);
    void cs_msg_register        (CRCNetClientC* client, CRCJson& msg);
    void cs_msg_change_pw       (CRCNetClientC* client, CRCJson& msg);
    void cs_msg_login_by_token  (CRCNetClientC* client, CRCJson& msg);

    void ss_msg_client_exit     (CRCNetClientC* client, CRCJson& msg);
    void ss_msg_user_exit       (CRCNetClientC* client, CRCJson& msg);
};

#endif