#ifndef _SVR_MKISO_H_
#define _SVR_MKISO_H_

#include "crc_net_client_c.h"

class MkisoServer
{
private:
    CRCNetClientC   m_csGate;
public:
    void Init();

    void Run();

    void Close();

private:
    void onopen_csGate          (CRCNetClientC* client, CRCJson& msg);

    void cs_package_iso_file    (CRCNetClientC* client, CRCJson& msg);
};
#endif //_SVR_MKISO_H_