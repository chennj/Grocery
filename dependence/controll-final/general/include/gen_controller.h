#ifndef _GEN_CONTROLLER_H_
#define _GEN_CONTROLLER_H_

#include "crc_net_server.h"
#include "crc_net_transfer.h"

class GenController
{
private:
    CRCNetServer    _netserver;
    CRCNetTransfer  _transfer;
public:
    void Init();

    void Close();

private:
    void cs_msg_heart(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg);

    void ss_reg_api(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg);

    void on_other_msg(CRCWorkServer* server, CRCNetClientS* client, std::string& cmd, CRCJson& msg);

    void on_client_leave(CRCNetClientS* client);
};

#endif //!_GEN_CONTROLLER_H_