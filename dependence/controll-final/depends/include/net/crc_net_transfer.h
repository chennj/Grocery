/**
 * 
 * author:  chenningjiang
 * desc:    连接分发处理器
 * 
 * */
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

        void on_broadcast_do(const char* pData, int len);
    private:
        std::vector<CRCNetClientS*> m_listeners;
        int index = 0;
    };

private:
    std::map<std::string, Listeners> m_msg_listeners;

public:
    void add(std::string cmd, CRCNetClientS* client);

    void del(CRCNetClientS* client);

    void del(std::string cmd, CRCNetClientS* client);

    int  on_net_msg_do(std::string& cmd, std::string& data);

    int  on_broadcast_do(const std::string& cmd, const std::string& data);
};
#endif