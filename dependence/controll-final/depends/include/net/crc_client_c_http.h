#ifndef _CRC_CLIENT_C_HTTP_H_
#define _CRC_CLIENT_C_HTTP_H_

#include "crc_client.h"
#include "crc_splitstring.hpp"
#include "crc_keystring.hpp"
#include <map>

class CRCClientCHttp : public CRCClient
{
public:
    CRCClientCHttp(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);
    virtual ~CRCClientCHttp();

    //是否有新的消息进来
    virtual bool hasMsg() override;

    // 0 消息不完整 继续等待消息
    // -1 不支持的消息类型
    // -2 异常消息
    int checkHttpResponse();
    
    //解析http消息
    //确定收到完整http消息的时候才能调用
    bool getResponseInfo();
    
    //解析url参数内容
    //可以是html页面
    //不过呢，我们只要能解析http api返回的json文本字符串
    //或者其它格式的字符串数据就可以了，例如xml格式
    void SplitUrlArgs(char* args);

    //弹出缓冲区第一个消息
    virtual void pop_front_msg() override;

    //是否带有参数
    bool has_args(const char* key);

    //是否有协议头
    bool has_header(const char* key);

    int args_getInt(const char* argName, int def);

    const char* args_getStr(const char* argName, const char* def);

    const char* header_getStr(const char* argName, const char* def);

    virtual void onRecvComplete();

    const char* content();

protected:
    int _headerLen = 0;
    int _bodyLen = 0;
    std::map<CRCIO::CRCKeyString, char*> _header_map;
    std::map<CRCIO::CRCKeyString, char*> _args_map;
    bool _keepalive = true;
};

#endif //!_CRC_CLIENT_C_HTTP_H_