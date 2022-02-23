/**
 * 
 * author:  chenningjiang
 * desc:    登录服务器（实际是一个连接客户端）
 * 
 * */
#ifndef _SVR_LOGIN_H_
#define _SVR_LOGIN_H_

#include "crc_net_client_c.h"

class LoginServer
{
private:
    CRCNetClientC m_csCtrl;
public:
    void Init();

    void Run();

    void Close();

private:
    void onopen_csCtrl(CRCNetClientC* client, CRCJson& msg);

    void cs_msg_login(CRCNetClientC* client, CRCJson& msg);
    void cs_msg_register(CRCNetClientC* client, CRCJson& msg);
    void cs_msg_change_pw(CRCNetClientC* client, CRCJson& msg);
};

#endif