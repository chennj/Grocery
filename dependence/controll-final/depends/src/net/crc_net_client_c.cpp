#include "crc_net_client_c.h"

bool 
CRCNetClientC::connect(const char* link_name,const char* url)
{
    _link_name = link_name;
    _url = url;

    int s_size = CRCConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
    int r_size = CRCConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

    _client.send_buff_size(s_size);
    _client.recv_buff_size(r_size);

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
        CRCLog_Info("websocket server say: %s", dataStr);

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

        //响应(对同一个msgId)
        bool is_resp = false;
        if (json.Get("is_resp", is_resp) && is_resp)
        {
            on_net_msg_do(msgId, json);
            return;
        }

        //请求
        bool is_req = false;
        if (json.Get("is_req", is_req) && is_req)
        {
            std::string cmd;
            if (!json.Get("cmd", cmd))
            {
                CRCLog_Error("not found key<%s>.", "cmd");
                return;
            }

            on_net_msg_do(cmd, json);
            return;
        }
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

bool 
CRCNetClientC::run(int microseconds)
{
    if (_client.isRun())
    {
        if (_time2heart.getElapsedSecond() > 5.0)
        {
            _time2heart.update();
            CRCJson json;
            //request("cs_msg_heart", json,[](CRCNetClientC* client, CRCJson& msg) {
            //    CRCLog_Info("CRCNetClientC::cs_msg_heart return: %s", msg("data").c_str());
            //});
            request("cs_msg_heart", json);
        }
        return _client.OnRun(microseconds);
    }

    if (_client.connect(_url.c_str()))
    {
        _time2heart.update();
        CRCLog_Warring("%s::CRCNetClientC::connect(%s) success.", _link_name.c_str(), _url.c_str());
        return true;
    }

    CRCThread::Sleep(1000);
    return false;
}

void 
CRCNetClientC::close()
{
    _client.Close();
}

void 
CRCNetClientC::reg_msg_call(std::string cmd, NetEventCall call)
{
    _map_msg_call[cmd] = call;
}

bool 
CRCNetClientC::on_net_msg_do(const std::string& cmd, CRCJson& msgJson)
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

bool 
CRCNetClientC::on_net_msg_do(int msgId, CRCJson& msgJson)
{
    auto itr = _map_request_call.find(msgId);
    if (itr != _map_request_call.end())
    {
        itr->second(this, msgJson);
        return true;
    }
    CRCLog_Info("%s::CRCNetClientC::on_net_msg_do not found msgId<%d>.", _link_name.c_str(), msgId);
    return false;    
}

void 
CRCNetClientC::request(const std::string& cmd, CRCJson& data)
{
    _time2heart.update();

    CRCJson msg;
    msg.Add("cmd", cmd);
    msg.Add("is_req", true, true);
    msg.Add("msgId", ++_msgId);
    msg.Add("time", CRCTime::system_clock_now());
    msg.Add("data", data);

    std::string retStr = msg.ToString();
    _client.writeText(retStr.c_str(), retStr.length());
}

void 
CRCNetClientC::request(const std::string& cmd, CRCJson& data, NetEventCall call)
{
    _time2heart.update();
    
    CRCJson msg;
    msg.Add("cmd", cmd);
    msg.Add("is_req", true, true);
    msg.Add("msgId", ++_msgId);
    _map_request_call[_msgId] = call;

    msg.Add("time", CRCTime::system_clock_now());
    msg.Add("data", data);

    std::string retStr = msg.ToString();
    _client.writeText(retStr.c_str(), retStr.length());
}

void 
CRCNetClientC::response(int msgId, std::string data)
{
    CRCJson ret;
    ret.Add("msgId", msgId);
    ret.Add("is_resp", true, true);
    ret.Add("time", CRCTime::system_clock_now());
    ret.Add("data", data);

    std::string retStr = ret.ToString();
    _client.writeText(retStr.c_str(), retStr.length());
}

void 
CRCNetClientC::response(CRCJson& msg, std::string data)
{
    int msgId = 0;
    if (!msg.Get("msgId", msgId))
    {
        CRCLog_Error("not found key<%s>.", "msgId");
        return;
    }

    CRCJson ret;
    ret.Add("msgId", msgId);
    ret.Add("is_resp", true, true);
    ret.Add("time", CRCTime::system_clock_now());
    ret.Add("data", data);

    std::string retStr = ret.ToString();
    _client.writeText(retStr.c_str(), retStr.length());
}

void 
CRCNetClientC::response(CRCJson& msg, CRCJson& ret)
{
    int msgId = 0;
    if (!msg.Get("msgId", msgId))
    {
        CRCLog_Error("not found key<%s>.", "msgId");
        return;
    }

    int clientId = 0;
    if (!msg.Get("clientId", clientId))
    {
        CRCLog_Error("not found key<%s>.", "clientId");
        return;
    }

    ret.Add("msgId", msgId);
    ret.Add("clientId", clientId);
    ret.Add("is_resp", true, true);
    ret.Add("time", CRCTime::system_clock_now());

    std::string retStr = ret.ToString();
    _client.writeText(retStr.c_str(), retStr.length());
}