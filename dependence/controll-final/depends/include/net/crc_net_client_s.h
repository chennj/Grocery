/**
 * 
 * author:  chenningjiang
 * desc:    用于{@CRCNetServer}的websocket客户端
 * 
 * */
#ifndef _CRC_NET_CLIENT_S_H_
#define _CRC_NET_CLIENT_S_H_

#include "crc_client_s_websocket.h"
#include "CJsonObject.hpp"
#include "crc_client_id.hpp"

using CRCJson = neb::CJsonObject;

class CRCNetClientS : public CRCClientSWebSocket
{
private:
    std::string m_link_name;
    std::string m_link_type = "client";
    std::string m_link_group = "0000";

    bool m_is_ss_link  = false;
    bool m_is_cc_link  = false;

    //
    std::string m_token;
    int64_t     m_userId = 0;
public:
    CRCNetClientS(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);

    const std::string& link_name()
    {
        return m_link_name;
    }

    void link_name(const std::string& str)
    {
        m_link_name = str;
    }

    const std::string& link_type()
    {
        return m_link_type;
    }

    void link_type(const std::string& str)
    {
        m_link_type = str;
    }

    const std::string& link_group()
    {
        return m_link_group;
    }

    void link_group(const std::string& str)
    {
        m_link_group = str;
    }

    bool is_ss_link()
    {
        return m_is_ss_link;
    }

    void is_ss_link(bool b)
    {
        m_is_ss_link = b;
    }

    bool is_cc_link()
    {
        return m_is_cc_link;
    }

    void is_cc_link(bool b)
    {
        m_is_cc_link = b;
    }

    const std::string& token()
    {
        return m_token;
    }

    void token(const std::string& str)
    {
        m_token = str;
    }

    int clientId()
    {
        return (int)this->sockfd();
    }

    int64_t userId()
    {
        return m_userId;
    }

    void userId(int64_t n)
    {
        m_userId = n;
    }

    bool is_login()
    {
        return m_userId != 0;
    }

    bool transfer(CRCJson& msg);

public:
    template<typename vT>
    void response(CRCJson& msg, const vT& data, int state = STATE_CODE_OK)
    {
        int msgId = 0;
        if (!msg.Get("msgId", msgId))
        {
            CRCLog_Error("not found key<msgId>.");
            return;
        }

        CRCJson ret;
        ret.Add("state", state);
        ret.Add("msgId", msgId);
        ret.Add("type",  MSG_TYPE_RESP);
        ret.Add("time",  CRCTime::system_clock_now());
        ret.Add("data",  data);

        transfer(ret);
    }

    template<typename vT>
    void resp_error(CRCJson& msg, const vT& data, int state = STATE_CODE_ERROR)
    {
        response(msg, data, state);
    }
};

#endif //!_CRC_NET_CLIENT_S_H_