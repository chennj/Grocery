#ifndef _CRC_NET_CLIENT_S_H_
#define _CRC_NET_CLIENT_S_H_

#include "crc_client_s_websocket.h"
#include "CJsonObject.hpp"

using CRCJson = neb::CJsonObject;

class CRCNetClientS : public CRCClientSWebSocket
{
private:
    std::string _link_name;
    std::string _link_type = "client";
    std::string _link_group = "0000";

    bool _is_ss_link = false;
public:
    CRCNetClientS(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);

    std::string& link_name();

    void link_name(std::string& str);

    std::string& link_type();

    void link_type(std::string& str);

    std::string& link_group();

    void link_group(std::string& str);

    bool is_ss_link();

    void is_ss_link(bool b);

    void response(int msgId, std::string data);

    void response(CRCJson& msg, std::string data);

    void response(CRCJson& msg, CRCJson& ret);
};

#endif //!_CRC_NET_CLIENT_S_H_