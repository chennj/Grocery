#include "crc_client_s_http.h"

CRCClientSHttp::CRCClientSHttp(SOCKET sockfd, int sendSize, int recvSize)
    : CRCClient(sockfd, sendSize,recvSize)
{}

CRCClientSHttp::~CRCClientSHttp()
{}

bool 
CRCClientSHttp::hasMsg()
{
    //完整的http消息一定超过20字节
    if (_recvBuff.dataLen() < 20)
        return false;

    int ret = checkHttpRequest();
    if (ret < 0)
        resp400BadRequest();
    return ret > 0;
}

int 
CRCClientSHttp::checkHttpRequest()
{
    //查找http消息结束标记
    char* temp = strstr(_recvBuff.data(), "\r\n\r\n");
    //未找到表示消息还不完整
    if (!temp)
        return 0;
    CRCLog_Info(_recvBuff.data());
    //偏移到消息结束位置
    //4=strlen("\r\n\r\n")
    temp += 4;
    //计算http请求消息的请求行+请求头长度
    _headerLen = temp - _recvBuff.data();
    //判断请求类型是否支持
    temp = _recvBuff.data();
    if (temp[0] == 'G' &&
        temp[1] == 'E' &&
        temp[2] == 'T')
    {
        _requestType = CRCClientSHttp::GET;
    }
    else if (
        temp[0] == 'P' &&
        temp[1] == 'O' &&
        temp[2] == 'S' &&
        temp[3] == 'T')
    {
        _requestType = CRCClientSHttp::POST;
        //POST需要计算请求体长度
        char* p1 = strstr(_recvBuff.data(), "Content-Length: ");
        //未找到表示格式错误
        //返回错误码或者直接关闭客户端连接
        if (!p1)
            return -2;
        //Content-Length: 1024\r\n
        //16=strlen("Content-Length: ")
        p1 += 16;
        char* p2 = strchr(p1, '\r');
        if (!p2)
            return -2;
        //计算数字长度
        int n = p2 - p1;
        //6位数 99万9999 上限100万字节， 就是1MB
        //我们目前是靠接收缓冲区一次性接收
        //所以数据上限是接收缓冲区大小减去_headerLen
        if (n > 6)
            return -2;
        char lenStr[7] = {};
        strncpy(lenStr, p1, n);
        _bodyLen = atoi(lenStr);
        //数据异常
        if(_bodyLen < 0)
            return -2;
        //消息数据超过了缓冲区可接收长度
        if (_headerLen + _bodyLen > _recvBuff.buffSize())
            return -2;
        //消息长度>已接收的数据长度，那么数据还没接收完
        if (_headerLen + _bodyLen > _recvBuff.dataLen())
            return 0;
    }
    else {
        _requestType = CRCClientSHttp::UNKOWN;
        return -1;
    }

    return _headerLen;
}

bool 
CRCClientSHttp::getRequestInfo()
{
    //判断是否已经收到了完整消息
    if (_headerLen <= 0)
        return false;
    
    char* pp = _recvBuff.data();
    pp[_headerLen-1] = '\0';

    CRCIO::CRCSplitString ss;
    ss.set(_recvBuff.data());
    //请求行示例："GET /login.php?a=5 HTTP/1.1\r\n"
    char* temp = ss.get("\r\n");
    if (temp)
    {
        _header_map["RequestLine"] = temp;
        request_args(temp);
    }

    //请求头示例："Connection: Keep-Alive\r\n"
    while (true)
    {
        //请求头每一行都有一个\r\n
        temp = ss.get("\r\n");
        if (temp)
        {
            //"Connection: Keep-Alive\0\n"
            CRCIO::CRCSplitString ss2;
            ss2.set(temp);
            //每个请求头字段都是"key: val\r\n"
            char* key = ss2.get(": ");
            char* val = ss2.get(": ");
            if (key && val)
            {
                //key = Connection
                //val = Keep-Alive
                _header_map[key] = val;
            }
        }
        else {
            break;
        }
    }

    //请求体
    if (_bodyLen > 0)
    {
        SplitUrlArgs(_recvBuff.data() + _headerLen);
    }
    //根据字段，做出相应处理
    const char* str = header_getStr("Connection", "");
    _keepalive = (0 == strcmp("keep-alive", str)
        || 0 == strcmp("Keep-Alive", str)
        || 0 == strcmp("Upgrade", str)
        );
    
    return true;
}

void 
CRCClientSHttp::SplitUrlArgs(char* args)
{
    CRCIO::CRCSplitString ss;
    ss.set(args);
    while (true)
    {
        char* temp = ss.get('&');
        if (temp)
        {
            CRCIO::CRCSplitString ss2;
            ss2.set(temp);
            char* key = ss2.get('=');
            char* val = ss2.get('=');
            if (key && val)
                _args_map[key] = val;
        }
        else {
            break;
        }
    }
}

bool 
CRCClientSHttp::request_args(char* requestLine)
{
    //requestLine="GET /login.php?a=5 HTTP/1.1"
    CRCIO::CRCSplitString ss;
    ss.set(requestLine);
    //requestLine="GET\0/login.php?a=5 HTTP/1.1"
    _method = ss.get(' ');
    if (!_method)
        return false;

    _url = ss.get(' ');
    if (!_url)
        return false;

    _httpVersion = ss.get(' ');
    if (!_httpVersion)
        return false;

    ss.set(_url);
    _url_path = ss.get('?');
    if (!_url_path)
        return false;

    _url_args = ss.get('?');
    if (!_url_args)
        return true;

    SplitUrlArgs(_url_args);

    return true;
}

void 
CRCClientSHttp::pop_front_msg()
{
    if (_headerLen > 0)
    {
        _recvBuff.pop(_headerLen + _bodyLen);
        _headerLen = 0;
        _bodyLen = 0;
        //清除本次消息请求的数据
        _args_map.clear();
        _header_map.clear();
    }
}

bool 
CRCClientSHttp::canWrite(int size)
{
    return _sendBuff.canWrite(size);
}

void 
CRCClientSHttp::resp400BadRequest()
{
    writeResponse("400 Bad Request", "Only support GET or POST.", 25);
}

void 
CRCClientSHttp::resp404NotFound()
{
    writeResponse("404 Not Found", "(^o^): 404!", 11);
}

void 
CRCClientSHttp::resp200OK(const char* bodyBuff, int bodyLen)
{
    writeResponse("200 OK", bodyBuff, bodyLen);
}

void 
CRCClientSHttp::writeResponse(const char* code, const char* bodyBuff, int bodyLen)
{
    //响应体内存长度
    char respBodyLen[32] = {};
    sprintf(respBodyLen, "Content-Length: %d\r\n", bodyLen);
    //响应行+响应头缓冲区 256字节足够了
    char response[256] = {};
    //响应行
    strcat(response, "HTTP/1.1 ");
    strcat(response, code);
    strcat(response, "\r\n");
    //响应头
    strcat(response, "Content-Type: text/html;charset=UTF-8\r\n");
    strcat(response, "Access-Control-Allow-Origin: *\r\n");
    strcat(response, "Connection: Keep-Alive\r\n");
    strcat(response, respBodyLen);
    strcat(response, "\r\n");
    //发送响应体
    SendData(response, strlen(response));
    SendData(bodyBuff, bodyLen);
}

bool 
CRCClientSHttp::url_compre(const char* str)
{
    return 0 == strcmp(_url_path, str);
}

bool 
CRCClientSHttp::has_args(const char* key)
{
    return _args_map.find(key) != _args_map.end();
}

bool 
CRCClientSHttp::has_header(const char* key)
{
    return _header_map.find(key) != _header_map.end();
}

int 
CRCClientSHttp::args_getInt(const char* argName, int def)
{
    auto itr = _args_map.find(argName);
    if (itr != _args_map.end())
    {
        def = atoi(itr->second);
    }
    return def;
}

const char* 
CRCClientSHttp::args_getStr(const char* argName, const char* def)
{
    auto itr = _args_map.find(argName);
    if (itr != _args_map.end())
    {
        return itr->second;
    }
    return def;
}

const char* 
CRCClientSHttp::header_getStr(const char* argName, const char* def)
{
    auto itr = _header_map.find(argName);
    if (itr != _header_map.end())
    {
        return itr->second;
    }
    else {
        return def;
    }
}

void 
CRCClientSHttp::onSendComplete()
{
    if (!_keepalive)
    {
        this->onClose();
    }
}