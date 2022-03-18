#include "crc_net_client_c.h"

CRCNetClientC::CRCNetClientC()
{
    m_ret_timeout.Add("type",   MSG_TYPE_RESP);
    m_ret_timeout.Add("msgId",  0);
    m_ret_timeout.Add("state",  STATE_CODE_TIMEOUT);
    m_ret_timeout.Add("data",   "request timeout");
}

bool 
CRCNetClientC::connect(const char* link_name,const char* url)
{
    m_link_name = link_name;
    m_url = url;

    int s_size = CRCConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
    int r_size = CRCConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

    m_client.send_buff_size(s_size);
    m_client.recv_buff_size(r_size);

    //do
    m_client.onopen = [this](CRCClientCWebSocket* pWSClient)
    {
        CRCLog_Info("%s::CRCNetClientC::connect(%s) success.", m_link_name.c_str(), m_url.c_str());
        CRCJson json;
        json.Add("link_name", m_link_name);
        json.Add("url", m_url);
        on_net_msg_do("onopen", json);
    };

    m_client.onmessage = [this](CRCClientCWebSocket* pWSClient)
    {
        CRCIO::WebSocketHeader& wsh = pWSClient->WebsocketHeader();
        if (wsh.opcode == CRCIO::opcode_PONG)
        {
            CRCLog_Info("websocket server say: PONG");
            return false;
        }

        auto pchar = pWSClient->fetch_data();
        std::string dataStr(pchar, wsh.len);
        //CRCLog_Info("websocket server say: %s", pchar);

        CRCJson json;
        if (!json.Parse(dataStr))
        {
            CRCLog_Error("json.Parse error : %s", json.GetErrMsg().c_str());
            return false;
        }

        int msg_type = 0;
        if (!json.Get("type", msg_type))
        {
            CRCLog_Error("not found key<type>");
            return false;
        }

        int msgId = 0;
        if (!json.Get("msgId", msgId))
        {
            CRCLog_Error("not found key<msgId>");
            return false;
        }

        time_t time = 0;
        if (!json.Get("time", time))
        {
            CRCLog_Error("not found key<time>");
            return false;
        }

        //响应
        if (MSG_TYPE_RESP ==  msg_type)
        {
            int msgId = 0;
            if (!json.Get("msgId", msgId))
            {
                CRCLog_Error("not found key<msgId>.");
                return false;
            }

            on_net_msg_do(msgId, json);
            return false;
        }

        //请求 or 推送
        if (MSG_TYPE_REQ        == msg_type ||
            MSG_TYPE_PUSH       == msg_type ||
            MSG_TYPE_BROADCAST  == msg_type)
        {
            if (on_other_push && MSG_TYPE_PUSH == msg_type)
            {//一般呢LinkGate才会有on_other_push
                do
                {
                    //没有clientId
                    //clientId和我相同的消息不需要on_other_push
                    int clientId = 0;
                    if (json.Get("clientId", clientId) && clientId == m_clientId)
                        break;

                    on_other_push(this, json);
                    return true;
                } while (false);
            }

            std::string cmd;
            if (!json.Get("cmd", cmd))
            {
                CRCLog_Error("not found key<cmd>.");
                return false;
            }

            on_net_msg_do(cmd, json);
            return true;
        }
    };

    m_client.onclose = [this](CRCClientCWebSocket* pWSClient)
    {
        CRCJson json;
        json.Add("link_name", m_link_name);
        json.Add("url", m_url);
        on_net_msg_do("onclose", json);
    };

    m_client.onerror = [this](CRCClientCWebSocket* pWSClient)
    {
        CRCJson json;
        json.Add("link_name", m_link_name);
        json.Add("url", m_url);
        on_net_msg_do("onerror", json);
    };

    return true;
}

void 
CRCNetClientC::connect(const char* link_name,const char* url, int s_size, int r_size)
{
    m_link_name = link_name;
    m_url = url;

    m_client.send_buff_size(s_size);
    m_client.recv_buff_size(r_size);

    //do
    m_client.onopen = [this](CRCClientCWebSocket* pWSClient)
    {
        //CRCLog_Info("%s::CRCNetClient::connect(%s) success.", _link_name.c_str(), _url.c_str());
        CRCJson json;
        json.Add("link_name",   m_link_name);
        json.Add("url",         m_url);
        on_net_msg_do("onopen", json);
    };

    m_client.onmessage = [this](CRCClientCWebSocket* pWSClient)
    {
        CRCIO::WebSocketHeader& wsh = pWSClient->WebsocketHeader();
        if (wsh.opcode == CRCIO::opcode_PONG)
        {
            CRCLog_Info("websocket server say: PONG");
            return;
        }

        auto pStr = pWSClient->fetch_data();
        std::string dataStr(pStr, wsh.len);
        //CRCLog_Info("websocket server say: %s", dataStr.c_str());

        neb::CJsonObject json;
        if (!json.Parse(dataStr))
        {
            CRCLog_Error("json.Parse error : %s", json.GetErrMsg().c_str());
            return;
        }

        int msg_type = 0;
        if (!json.Get("type", msg_type))
        {
            CRCLog_Error("not found key<type>.");
            return;
        }

        //响应
        if (MSG_TYPE_RESP ==  msg_type)
        {
            int msgId = 0;
            if (!json.Get("msgId", msgId))
            {
                CRCLog_Error("not found key<msgId>.");
                return;
            }

            on_net_msg_do(msgId, json);
            return;
        }

        //请求 or 推送
        if (MSG_TYPE_REQ        == msg_type ||
            MSG_TYPE_PUSH       == msg_type ||
            MSG_TYPE_BROADCAST  == msg_type)
        {
            if (on_other_push && MSG_TYPE_PUSH == msg_type)
            {//一般呢LinkGate才会有on_other_push
                do
                {
                    //没有clientId
                    //clientId和我相同的消息不需要on_other_push
                    int clientId = 0;
                    if (json.Get("clientId", clientId) && clientId == m_clientId)
                        break;

                    on_other_push(this, json);
                    return;
                } while (false);
            }

            std::string cmd;
            if (!json.Get("cmd", cmd))
            {
                CRCLog_Error("not found key<cmd>.");
                return;
            }

            on_net_msg_do(cmd, json);
            return;
        }
    };

    m_client.onclose = [this](CRCClientCWebSocket* pWSClient)
    {
        CRCJson json;
        json.Add("link_name",   m_link_name);
        json.Add("url",         m_url);

        on_net_msg_do("onclose", json);
    };

    m_client.onerror = [this](CRCClientCWebSocket* pWSClient)
    {
        neb::CJsonObject json;
        json.Add("link_name",   m_link_name);
        json.Add("url",         m_url);
        
        on_net_msg_do("onerror", json);
    };   
}

bool 
CRCNetClientC::run(int microseconds)
{
    if (m_client.isRun())
    {
        check_timeout();

        if (m_time2heart.getElapsedSecond() > 5.0)
        {
            m_time2heart.update();
            CRCJson json;
            //request("cs_msg_heart", json,[](CRCNetClientC* client, CRCJson& msg) {
            //    CRCLog_Info("CRCNetClientC::cs_msg_heart return: %s", msg("data").c_str());
            //});
            request("cs_msg_heart", json);
        }
        return m_client.OnRun(microseconds);
    }

    if (m_client.connect(m_url.c_str()))
    {
        m_time2heart.update();
        CRCLog_Warring("%s::CRCNetClientC::connect(%s) success.", m_link_name.c_str(), m_url.c_str());
        return true;
    }

    CRCThread::Sleep(1000);
    return false;
}

void 
CRCNetClientC::close()
{
    m_client.Close();
}

void 
CRCNetClientC::to_close()
{
    m_client.Close();
}

void
CRCNetClientC::check_timeout()
{
    //如果_timeout_dt为0  就不检测超时
    if (0 == m_timeout_dt)
        return;

    time_t now = CRCTime::system_clock_now();
    for (auto itr = m_map_request_call.begin(); itr != m_map_request_call.end(); )
    {
        if (now - itr->second.dt >= m_timeout_dt)
        {
            //该请求触发超时
            itr->second.callFun(this, m_ret_timeout);
            //移除该请求的响应回调
            auto itrOld = itr;
            ++itr;
            m_map_request_call.erase(itrOld);
            continue;
        }
        ++itr;
    }
}   

void 
CRCNetClientC::reg_msg_call(std::string cmd, NetEventCall call)
{
    m_map_msg_call[cmd] = call;
}

bool 
CRCNetClientC::on_net_msg_do(const std::string& cmd, CRCJson& msgJson)
{
    auto itr = m_map_msg_call.find(cmd);
    if (itr != m_map_msg_call.end())
    {
        itr->second(this, msgJson);
        return true;
    }
    CRCLog_Info("%s::CRCNetClientC::on_net_msg_do not found cmd<%s>.", m_link_name.c_str(),cmd.c_str());
    return false;
}

bool 
CRCNetClientC::on_net_msg_do(int msgId, CRCJson& msgJson)
{
    auto itr = m_map_request_call.find(msgId);
    if (itr != m_map_request_call.end())
    {
        itr->second.callFun(this, msgJson);
        m_map_request_call.erase(itr);
        return true;
    }
    CRCLog_Info("%s::CRCNetClientC::on_net_msg_do not found msgId<%d>.", m_link_name.c_str(), msgId);
    return false;    
}

bool 
CRCNetClientC::transfer(CRCJson& msg)
{
    std::string retStr = msg.ToString();
    if (SOCKET_ERROR == m_client.writeText(retStr.c_str(), retStr.length()))
    {
        CRCLog_Error("INetClient::transfer::writeText SOCKET_ERROR.");
        return false;
    }
    m_time2heart.update();
    return true;
}

bool 
CRCNetClientC::request(CRCJson& msg, NetEventCall call)
{
    //置换msgId
    int msgId = 0;
    if (!msg.Get("msgId", msgId))
    {
        msg.Add("msgId", ++m_msgId);
    }
    else {
        msg.Replace("msgId", ++m_msgId);
    }

    //转发
    if (!transfer(msg))
    {
        return false;
    }

    //记录回调
    if (call != nullptr)
    {
        NetEventCallData calldata;
        calldata.callFun            = call;
        calldata.dt                 = CRCTime::system_clock_now();
        m_map_request_call[m_msgId] = calldata;
    }
    return true;
}


