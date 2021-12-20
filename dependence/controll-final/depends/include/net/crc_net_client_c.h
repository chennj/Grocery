#ifndef _CRC_NET_CLIENT_C_H_
#define _CRC_NET_CLIENT_C_H_

#include "crc_easy_websocket_client.h"
#include "CJsonObject.hpp"
#include "crc_config.h"
#include "crc_timestamp.h"

using CRCJson = neb::CJsonObject;

class CRCNetClientC
{
private:
    CRCEasyWebSocketClient _client;
    //心跳计时器
    CRCTimestamp _time2heart;
    //
    std::string _link_name;
    //
    std::string _url;
    //
    int _msgId = 0;
private:
    //回调函数定义
    typedef std::function<void(CRCNetClientC*, CRCJson&)> NetEventCall;
    //消息回调函数
    std::map<std::string, NetEventCall> _map_msg_call;
    //请求返回回调函数
    std::map<int, NetEventCall> _map_request_call;
public:
    //并不正真连接，连接放在run()里面
    //这里只是注册一些lamdba函数
    bool connect(const char* link_name,const char* url);
    //启动连接，断线重连，空闲时发送心跳
    bool run(int microseconds = 1);
    //关闭客户端（即通道）
    void close();
    //注册命令的回调响应函数
    void reg_msg_call(std::string cmd, NetEventCall call);
    //对同一个命令的响应
    bool on_net_msg_do(const std::string& cmd, CRCJson& msgJson);   
    //对同一个请求消息的响应
    bool on_net_msg_do(int msgId, CRCJson& msgJson);
    //不带回调的请求
    void request(const std::string& cmd, CRCJson& data);
    //带回调的请求，针对同一个消息ID
    void request(const std::string& cmd, CRCJson& data, NetEventCall call);
    //响应请求：消息ID
    void response(int msgId, std::string data);
    //响应请求：带消息数据
    void response(CRCJson& msg, std::string data);
    //响应请求：json对象
    void response(CRCJson& msg, CRCJson& ret);
};

#endif //!_CRC_NET_CLIENT_C_H_


