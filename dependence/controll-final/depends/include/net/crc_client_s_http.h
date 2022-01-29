#ifndef _CRC_CLIENT_S_HTTP_H_
#define _CRC_CLIENT_S_HTTP_H_

#include "crc_client.h"
#include "crc_splitstring.hpp"
#include "crc_keystring.hpp"
#include <map>

class CRCClientSHttp : public CRCClient
{
public:
    enum RequestType
    {
        GET = 10,
        POST,
        UNKOWN
    };

public:
    CRCClientSHttp(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);
    virtual ~CRCClientSHttp();

    virtual bool hasMsg() override;

    // 0 消息不完整 继续等待消息
    // -1 不支持的消息类型
    // -2 异常消息
    int checkHttpRequest();
    
    //解析http消息
    //确定收到完整http消息的时候才能调用
    bool getRequestInfo();
    
    //解析url参数内容
    //可以是html页面
    //不过呢，我们只要能解析http api返回的json文本字符串
    //或者其它格式的字符串数据就可以了，例如xml格式
    void SplitUrlArgs(char* args);

    bool request_args(char* requestLine);

    virtual void pop_front_msg() override;

    bool canWrite(int size);

    void resp400BadRequest();
    void resp404NotFound();
    void resp200OK(const char* bodyBuff, int bodyLen);

    void writeResponse(const char* code, const char* bodyBuff, int bodyLen);

    inline char* url()
    {
        return _url_path;
    }

    bool url_compre(const char* str);

    bool has_args(const char* key);

    bool has_header(const char* key);

    int args_getInt(const char* argName, int def);

    const char* args_getStr(const char* argName, const char* def);

    const char* header_getStr(const char* argName, const char* def);

    virtual void onSendComplete() override;
    
protected:
    int _headerLen = 0;
    int _bodyLen = 0;
    std::map<CRCIO::CRCKeyString, char*> _header_map;
    std::map<CRCIO::CRCKeyString, char*> _args_map;
    RequestType _requestType = CRCClientSHttp::UNKOWN;
    char* _method;
    char* _url;
    char* _url_path;
    char* _url_args;
    char* _httpVersion;
    bool _keepalive = true;
};

#endif //!_CRC_CLIENT_S_HTTP_H_