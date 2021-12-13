#ifndef _CRC_NET_TRANSFER_H_
#define _CRC_NET_TRANSFER_H_

#include "crc_net_client_s.h"
#include "CJsonObject.hpp"
#include <vector>

//消息分发
class CRCNetTransfer
{
    //服务监听者(服务集群)
    class Listeners
    {
    public:
        void add(CRCNetClientS* client);

        void del(CRCNetClientS* client);

        CRCNetClientS* get();
    private:
        std::vector<CRCNetClientS*> _listeners;
        int index = 0;
    };

private:
    std::map<std::string, Listeners> _msg_listeners;

public:
    void add(std::string cmd, CRCNetClientS* client);

    void del(CRCNetClientS* client);

    void del(std::string cmd, CRCNetClientS* client);

    bool on_net_msg_do(std::string& cmd, std::string& data);
};
#endif