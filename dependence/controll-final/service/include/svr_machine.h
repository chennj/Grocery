#ifndef _SVR_MACHINE_H_
#define _SVR_MACHINE_H_

#include "crc_net_client_c.h"
#include "crc_easy_txt_client.h"
#include "crc_thread.h"

class MachineServer
{
private:
    CRCNetClientC _csCtrl;
    CRCEasyTxtClient _csMachine;
    CRCThread _thread;

public:
    void Init();

    void Run();

    void Close();

    void MachineLoop(CRCThread* pThread);

    inline CRCEasyTxtClient& machine(){return _csMachine;}

private:
    void onopen_csCtrl(CRCNetClientC* client, CRCJson& msg);

    void cs_machine_mailbox(CRCNetClientC* client, CRCJson& msg);
};

#endif