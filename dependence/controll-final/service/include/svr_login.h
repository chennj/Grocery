#ifndef _SVR_LOGIN_H_
#define _SVR_LOGIN_H_

#include "crc_net_client_c.h"

class LoginServer
{
private:
    CRCNetClientC _csCtrl;
public:
    void Init();

    void Run();

    void Close();

private:
    void onopen_csCtrl(CRCNetClientC* client, CRCJson& msg);

    void cs_msg_login(CRCNetClientC* client, CRCJson& msg);
};

#endif