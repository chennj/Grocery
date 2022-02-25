#ifndef _CRC_GATE_SERVER_H_
#define _CRC_GATE_SERVER_H_

#include "crc_net_server.h"
#include "crc_net_transfer.h"

class GateServer
{
private:
    CRCNetServer    m_netserver;
    CRCNetTransfer  m_transfer;
public:
    void Init();

    void Close();

private:
    void cs_msg_heart(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg);

    void ss_reg_api(CRCWorkServer* server, CRCNetClientS* client, CRCJson& msg);

    void on_other_msg(CRCWorkServer* server, CRCNetClientS* client, std::string& cmd, CRCJson& msg);

    void on_broadcast_msg(CRCWorkServer* server, CRCNetClientS* client, std::string& cmd, CRCJson& msg);

    void on_client_leave(CRCNetClientS* client);

private:
    template<typename vT>
    void broadcast(const std::string& cmd, const vT& data)
    {
        CRCJson ret;
        ret.Add("cmd",  cmd);
        ret.Add("type", MSG_TYPE_BROADCAST);
        ret.Add("time", CRCTime::system_clock_now());
        ret.Add("data", data);

        auto str = ret.ToString();
        m_transfer.on_broadcast_do(cmd, str);
    }
};

#endif