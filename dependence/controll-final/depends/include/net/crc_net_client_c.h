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
    bool connect(const char* link_name,const char* url)
    {
        _link_name = link_name;
        _url = url;

        int s_size = CRCConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
        int r_size = CRCConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

        _client.send_buff_size(s_size);
        _client.recv_buff_size(r_size);

        if (!_client.connect(url))
        {
            CRCLog_Warring("%s::CRCNetClientC::connect(%s) failed.", _link_name.c_str(), _url.c_str());
            return false;
        }

        //do
        _client.onopen = [this](CRCClientCWebSocket* pWSClient)
        {
            CRCLog_Info("%s::CRCNetClientC::connect(%s) success.", _link_name.c_str(), _url.c_str());
            CRCJson json;
            json.Add("link_name", _link_name);
            json.Add("url", _url);
            on_net_msg_do("onopen", json);
        };

        _client.onmessage = [this](CRCClientCWebSocket* pWSClient)
        {
            CRCIO::WebSocketHeader& wsh = pWSClient->WebsocketHeader();
            if (wsh.opcode == CRCIO::opcode_PONG)
            {
                CRCLog_Info("websocket server say: PONG");
                return;
            }

            auto dataStr = pWSClient->fetch_data();
            //CRCLog_Info("websocket server say: %s", dataStr);

            CRCJson json;
            if (!json.Parse(dataStr))
            {
                CRCLog_Error("json.Parse error : %s", json.GetErrMsg().c_str());
                return;
            }

            int msgId = 0;
            if (!json.Get("msgId", msgId))
            {
                CRCLog_Error("not found key<%s>.", "msgId");
                return;
            }

            time_t time = 0;
            if (!json.Get("time", time))
            {
                CRCLog_Error("not found key<%s>.", "time");
                return;
            }

            std::string cmd;
            if (!json.Get("cmd", cmd))
            {
                CRCLog_Error("not found key<%s>.", "cmd");
                return;
            }

            std::string data;
            if (!json.Get("data", data))
            {
                CRCLog_Error("not found key<%s>.", "data");
                return;
            }

            on_net_msg_do(cmd, json);
        };

        _client.onclose = [this](CRCClientCWebSocket* pWSClient)
        {
            CRCJson json;
            json.Add("link_name", _link_name);
            json.Add("url", _url);
            on_net_msg_do("onclose", json);
        };

        _client.onerror = [this](CRCClientCWebSocket* pWSClient)
        {
            CRCJson json;
            json.Add("link_name", _link_name);
            json.Add("url", _url);
            on_net_msg_do("onerror", json);
        };

    }

    bool run(int microseconds = 1)
    {
        return _client.OnRun(microseconds);
    }

    void close()
    {
        _client.Close();
    }

    void reg_msg_call(std::string cmd, NetEventCall call)
    {
        _map_msg_call[cmd] = call;
    }

    bool on_net_msg_do(const std::string& cmd, CRCJson& msgJson)
    {
        auto itr = _map_msg_call.find(cmd);
        if (itr != _map_msg_call.end())
        {
            itr->second(this, msgJson);
            return true;
        }
        CRCLog_Info("%s::CRCNetClientC::on_net_msg_do not found cmd<%s>.", _link_name.c_str(),cmd.c_str());
        return false;
    }


    void request(const std::string& cmd, CRCJson& data)
    {
        CRCJson msg;
        msg.Add("cmd", cmd);
        msg.Add("msgId", ++msgId);
        msg.Add("time", CRCTime::system_clock_now());
        msg.Add("data", data);

        std::string retStr = msg.ToString();
        _client.writeText(retStr.c_str(), retStr.length());
    }

    void response(int msgId, std::string data)
    {
        CRCJson ret;
        ret.Add("msgId", msgId);
        ret.Add("time", CRCTime::system_clock_now());
        ret.Add("data", data);

        std::string retStr = ret.ToString();
        _client.writeText(retStr.c_str(), retStr.length());
    }

    void response(CRCJson& msg, std::string data)
    {
        int msgId = 0;
        if (!msg.Get("msgId", msgId))
        {
            CRCLog_Error("not found key<%s>.", "msgId");
            return;
        }

        CRCJson ret;
        ret.Add("msgId", msgId);
        ret.Add("time", CRCTime::system_clock_now());
        ret.Add("data", data);

        std::string retStr = ret.ToString();
        _client.writeText(retStr.c_str(), retStr.length());
    }

    void response(CRCJson& msg, CRCJson& ret)
    {
        int msgId = 0;
        if (!msg.Get("msgId", msgId))
        {
            CRCLog_Error("not found key<%s>.", "msgId");
            return;
        }

        ret.Add("msgId", msgId);
        ret.Add("time", CRCTime::system_clock_now());

        std::string retStr = ret.ToString();
        _client.writeText(retStr.c_str(), retStr.length());
    }
};

#endif //!_CRC_NET_CLIENT_C_H_


