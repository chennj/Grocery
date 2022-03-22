/**
 * 
 * author:  chenningjiang
 * desc:    使用websocket客户端的连接客户端处理器
 * 
 * */
#ifndef _CRC_NET_CLIENT_C_H_
#define _CRC_NET_CLIENT_C_H_

#include "crc_easy_websocket_client.h"
#include "CJsonObject.hpp"
#include "crc_config.h"
#include "crc_timestamp.h"
#include "crc_client_id.hpp"

using CRCJson = neb::CJsonObject;

class CRCNetClientC
{
private:
    CRCJson                 m_ret_timeout;
    //
    CRCEasyWebSocketClient  m_client;
    //心跳计时器
    CRCTimestamp            m_time2heart;
    //从发送请求起_timeout_dt毫秒会触发超时
    time_t                  m_timeout_dt = 15000;
    //
    std::string             m_link_name;
    //
    std::string             m_url;
    //
    int                     m_msgId = 0;
    //
    std::string             m_groupId = "0000";
    //
    int                     m_clientId = 0;
private:
    //回调函数定义
    typedef std::function<void(CRCNetClientC*, CRCJson&)> NetEventCall;
    //
    struct NetEventCallData {
            NetEventCall callFun = nullptr;
            time_t dt = 0;
	};
    //消息回调函数
    std::map<std::string, NetEventCall>     m_map_msg_call;
    //请求返回回调函数
    std::map<int, NetEventCallData>         m_map_request_call;
public:
    //处理不是给我的推送消息
    NetEventCall                            on_other_push = nullptr;
public:
    CRCNetClientC();

    //并不正真连接，连接放在run()里面
    //这里只是注册一些lamdba函数
    bool connect(const char* link_name,const char* url);
    //
    void connect(const char* link_name,const char* url, int s_size, int r_size);
    //启动连接，断线重连，空闲时发送心跳
    bool run(int microseconds = 1);
    //关闭客户端（即通道）
    void close();
    //
    void to_close();
    //检查超时
    void check_timeout(); 
    //注册命令的回调响应函数
    void reg_msg_call(std::string cmd, NetEventCall call);
    //对同一个命令的响应
    bool on_net_msg_do(const std::string& cmd, CRCJson& msgJson);   
    //对同一个请求消息的响应
    bool on_net_msg_do(int msgId, CRCJson& msgJson);
    //
    bool transfer(CRCJson& msg);
    //请求
    bool request(CRCJson& msg, NetEventCall call);

    //读写groupid
    inline void set_groupId(std::string groupId){m_groupId = groupId;}
    inline std::string& get_groupId(){return m_groupId;}
    //读写clientId
    inline int  get_clientId(){return m_clientId;}
    inline void set_clientId(int n){m_clientId = n;}
    //设置超时
    inline void timeout_dt(time_t dt){m_timeout_dt = dt;}

public:
    template<typename vT>
    bool request(const std::string& cmd, const std::string& groupId, const vT& data)
    {
        CRCJson msg;
        msg.Add("cmd",      cmd);
        msg.Add("type",     MSG_TYPE_REQ);
        msg.Add("groupId",  groupId);
        msg.Add("msgId",    ++m_msgId);
        msg.Add("time",     CRCTime::system_clock_now());
        msg.Add("data",     data);

        return transfer(msg);
    }

    template<typename vT>
    bool request(const std::string& cmd, const vT& data)
    {
        CRCJson msg;
        msg.Add("cmd",      cmd);
        msg.Add("type",     MSG_TYPE_REQ);
        msg.Add("groupId",  get_groupId());
        msg.Add("msgId",    ++m_msgId);
        msg.Add("time",     CRCTime::system_clock_now());
        msg.Add("data",     data);

        return transfer(msg);
    }

    template<typename vT>
    bool request(const std::string& cmd, const std::string& groupId, const vT& data, NetEventCall call)
    {
        if (!request(cmd, groupId, data))
        {
            return false;
        }

        if (call != nullptr)
        {
            NetEventCallData calldata;
            calldata.callFun            = call;
            calldata.dt                 = CRCTime::system_clock_now();
            m_map_request_call[m_msgId] = calldata;

            CRCLog_Info("callback msgId <%d>", m_msgId);
        }
        return true;
    }

    template<typename vT>
    bool request(const std::string& cmd, const vT& data, NetEventCall call)
    {
        if (!request(cmd,data))
        {
            return false;
        }

        if (call != nullptr)
        {
            NetEventCallData calldata;
            calldata.callFun            = call;
            calldata.dt                 = CRCTime::system_clock_now();
            m_map_request_call[m_msgId] = calldata;
        }
        return true;
    }

    template<typename vT>
    void response(int clientId, int msgId, const vT& data, int state = STATE_CODE_OK)
    {
        CRCJson ret;
        ret.Add("state",    state);
        ret.Add("msgId",    msgId);
        ret.Add("groupId",  get_groupId());
        ret.Add("clientId", clientId);
        ret.Add("type",     MSG_TYPE_RESP);
        ret.Add("time",     CRCTime::system_clock_now());
        ret.Add("data",     data);

        transfer(ret);
    }

    template<typename vT>
    void response(CRCJson& msg, const vT& data, int state = STATE_CODE_OK)
    {
        //通用基础字段获取与验证
        int clientId = 0;
        if (!msg.Get("clientId", clientId))
        {
            CRCLog_Error("INetClient::transfer::response not found key<clientId>.");
            return;
        }

        int msgId = 0;
        if (!msg.Get("msgId", msgId))
        {
            CRCLog_Error("INetClient::transfer::response not found key<msgId>.");
            return;
        }

        CRCLog_Info("response clientId <%d>, msgId <%d>", clientId, msgId);

        response(clientId, msgId, data, state);
    }

    template<typename vT>
    void resp_error(CRCJson& msg, const vT& data, int state = STATE_CODE_ERROR)
    {
        response(msg, data, state);
    }

    template<typename vT>
    void push(int clientId, const std::string& cmd, const vT& data, int state = STATE_CODE_OK)
    {
        CRCJson ret;
        ret.Add("state",    state);
        ret.Add("clientId", clientId);
        ret.Add("cmd",      cmd);
        ret.Add("type",     MSG_TYPE_PUSH);
        ret.Add("time",     CRCTime::system_clock_now());
        ret.Add("data",     data);

        transfer(ret);
    }

    template<typename vT>
    void push(const std::vector<int64_t>& clients, const std::string& cmd, const vT& data, int state = STATE_CODE_OK)
    {
        CRCJson ret;
        ret.Add("state", state);
        
        ret.Add("cmd",  cmd);
        ret.Add("type", MSG_TYPE_PUSH_S);
        ret.Add("time", CRCTime::system_clock_now());
        ret.Add("data", data);

        ret.AddEmptySubArray("clients");

        auto length = clients.size();
        for (size_t i = 0; i < length; i++)
        {
            ret["clients"].Add((int64)clients[i]);
        }

        transfer(ret);
    }

    template<typename vT>
    void broadcast(const std::string& cmd, const vT& data)
    {
        CRCJson ret;
        ret.Add("cmd",  cmd);
        ret.Add("type", MSG_TYPE_BROADCAST);
        ret.Add("time", CRCTime::system_clock_now());
        ret.Add("data", data);

        transfer(ret);
    }
};

#endif //!_CRC_NET_CLIENT_C_H_


