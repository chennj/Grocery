#ifndef _CRC_NET_CLIENT_C_H_
#define _CRC_NET_CLIENT_C_H_

#include "crc_easy_websocket_client.h"
#include "CJsonObject.hpp"
#include "crc_config.h"

using CRCJson = neb::CJsonObject;

class CRCNetClientC
{
private:
    CRCEasyWebSocketClient _client;
    //
    std::string _link_name;
    std::string _url;
    //
    int msgId = 0;
private:
    typedef std::function<void(CRCNetClientC*, CRCJson&)> NetEventCall;
    std::map<std::string, NetEventCall> _map_msg_call;
public:
    bool connect(const char* link_name,const char* url);

    bool run(int microseconds = 1);

    void close();

    void reg_msg_call(std::string cmd, NetEventCall call);

    bool on_net_msg_do(const std::string& cmd, CRCJson& msgJson);

    void request(const std::string& cmd, CRCJson& data);

    void response(int msgId, std::string data);

    void response(CRCJson& msg, std::string data);

    void response(CRCJson& msg, CRCJson& ret);
};

#endif //!_CRC_NET_CLIENT_C_H_


