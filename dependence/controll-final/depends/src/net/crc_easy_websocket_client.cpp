#include "crc_easy_websocket_client.h"
#include "crc_base64.h"

CRCEasyWebSocketClient::CRCEasyWebSocketClient()
{
    CRCNetWork::Init();
}

CRCClient* 
CRCEasyWebSocketClient::makeClientObj(SOCKET cSock, int sendSize, int recvSize)
{
    _pWSClient = new CRCClientCWebSocket(cSock, sendSize, recvSize);
    return _pWSClient;
}

void 
CRCEasyWebSocketClient::OnDisconnect() 
{
    if (onclose)
    {
        //CRCClientCWebSocket* pWSClient = dynamic_cast<CRCClientCWebSocket*>(_pClient);
        if(_pWSClient)
            onclose(_pWSClient);
    }
}

void 
CRCEasyWebSocketClient::OnNetMsg(CRCDataHeader* header)
{
    CRCClientCWebSocket* pWSClient = dynamic_cast<CRCClientCWebSocket*>(_pClient);
    if (!pWSClient)
        return;

    if (clientState_join == pWSClient->state())
    {
        if (!pWSClient->getResponseInfo())
            return;

        if (handshake())
        {
            CRCLog_Info("CRCClientCWebSocket::handshake, Good.");
            pWSClient->state(clientState_run);

            if (onopen)
            {
                onopen(pWSClient);
            }
        }
        else {
            CRCLog_Warring("CRCClientCWebSocket::handshake, Bad.");
            pWSClient->onClose();
            if (onerror)
            {
                onerror(pWSClient);
            }
        }
    }
    else if (clientState_run == pWSClient->state()) {
        CRCIO::WebSocketHeader& wsh = pWSClient->WebsocketHeader();
        if (wsh.opcode == CRCIO::opcode_PING)
        {
            pWSClient->pong();
        }
        else {
            //处理数据帧
            if (onmessage)
            {
                onmessage(pWSClient);
            }
        }
    }
}

bool 
CRCEasyWebSocketClient::handshake()
{

    CRCClientCWebSocket* pWSClient = dynamic_cast<CRCClientCWebSocket*>(_pClient);
    if (!pWSClient)
        return false;

    auto strUpgrade = pWSClient->header_getStr("Upgrade", "");
    if (0 != strcmp(strUpgrade, "websocket"))
    {
        CRCLog_Error("CRCClientCWebSocket::handshake, not found Upgrade:websocket");
        return false;
    }

    auto sKeyAccept = pWSClient->header_getStr("Sec-WebSocket-Accept", nullptr);
    if (!sKeyAccept)
    {
        CRCLog_Error("CRCClientCWebSocket::handshake, not found Sec-WebSocket-Key");
        return false;
    }

    std::string cKeyAccept = _cKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    unsigned char strSha1[20] = {};
    CRCIO::SHA1_String((const unsigned char*)cKeyAccept.c_str(), cKeyAccept.length(), strSha1);

    cKeyAccept = CRCIO::Base64Encode(strSha1, 20);

    if (cKeyAccept != sKeyAccept)
    {
        CRCLog_Error("CRCClientCWebSocket::handshake, cKeyAccept != sKeyAccept!");
        return false;
    }
    return true;
}

bool 
CRCEasyWebSocketClient::connect(const char* httpurl)
{
    deatch_http_url(httpurl);
    if (0 == hostname2ip(_host.c_str(), _port.c_str()))
    {
        _pClient->state(clientState_join);

        _cKey = "1234567890123456";
        _cKey = CRCIO::Base64Encode((const unsigned char*)_cKey.c_str(), _cKey.length());

        url2get(_host.c_str(), _path.c_str(), _args.c_str());
        return true;
    }
    return false;
}

int 
CRCEasyWebSocketClient::hostname2ip(const char* hostname, const char* port)
{
    if (!hostname)
    {
        CRCLog::Warring("hostname2ip(hostname is null ptr).");
        return -1;
    }

    if (!port)
    {
        CRCLog::Warring("hostname2ip(port is null ptr).");
        return -1;
    }

    unsigned short port_ = 80;
    if (port, strlen(port) > 0)
        port_ = atoi(port);

    //主机名和端口号不变，就不重新连接服务端
    if (isRun())
        return 0;

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;
    hints.ai_flags = AI_ALL;
    addrinfo* pAddrList = nullptr;
    int ret = getaddrinfo(hostname, nullptr, &hints, &pAddrList);
    if (0 != ret)
    {
        CRCLog::PError("%s getaddrinfo", hostname);
        freeaddrinfo(pAddrList);
        return ret;
    }

    char ipStr[256] = {};
    for (auto pAddr = pAddrList; pAddr != nullptr; pAddr = pAddr->ai_next)
    {
        ret = getnameinfo(pAddr->ai_addr, pAddr->ai_addrlen, ipStr, 255, nullptr, 0, NI_NUMERICHOST);
        if (0 != ret)
        {
            CRCLog::PError("%s getnameinfo", hostname);
            continue;
        }
        else {
            if (pAddr->ai_family == AF_INET6)
                CRCLog::Info("%s ipv6: %s", hostname, ipStr);
            else if (pAddr->ai_family == AF_INET)
                CRCLog::Info("%s ipv4: %s", hostname, ipStr);
            else {
                CRCLog::Info("%s addr: %s", hostname, ipStr);
                continue;
            }

            if (connet2ip(pAddr->ai_family, ipStr, port_))
            {
                return 0;
            }
        }
    }

    freeaddrinfo(pAddrList);
    return -1;
}

int 
CRCEasyWebSocketClient::writeText(const char* pData, int len)
{
    if (_pWSClient)
        return _pWSClient->writeText(pData, len);
    return 0;
}

void 
CRCEasyWebSocketClient::send_buff_size(int n)
{
    _nSendBuffSize = n;
}

void 
CRCEasyWebSocketClient::recv_buff_size(int n)
{
    _nRecvBuffSize = n;
}

void 
CRCEasyWebSocketClient::url2get(const char* host, const char* path, const char* args)
{
    std::string msg = "GET ";

    if (path && strlen(path) > 0)
        msg += path;
    else
        msg += "/";

    if (args && strlen(args) > 0)
    {
        msg += "?";
        msg += args;
    }

    msg += " HTTP/1.1\r\n";

    msg += "Host: ";
    msg += host;
    msg += "\r\n";

    //msg += "Connection: keep-alive\r\n";
    msg += "Accept: */*\r\n";

    msg += "Origin: ";
    msg += host;
    msg += "\r\n";
    ////
    msg += "Connection: Upgrade\r\n";
    msg += "Upgrade: websocket\r\n";
    msg += "Sec-WebSocket-Version: 13\r\n";

    msg += "Sec-WebSocket-Key: ";
    msg += _cKey;
    msg += "\r\n";
    ////

    msg += "\r\n";

    SendData(msg.c_str(), msg.length());
}

bool 
CRCEasyWebSocketClient::connet2ip(int af, const char* ip, unsigned short port)
{
    if (!ip)
        return false;

    if (INVALID_SOCKET == InitSocket(af, _nSendBuffSize, _nRecvBuffSize))
        return false;

    if (SOCKET_ERROR == Connect(ip, port))
        return false;

    CRCLog_Info("connet2ip(%s,%d)", ip, port);
    return true;
}

void 
CRCEasyWebSocketClient::deatch_http_url(std::string httpurl)
{
    _httpType.clear();
    _host.clear();
    _port.clear();
    _path.clear();
    _args.clear();

    auto pos1 = httpurl.find("://");
    if (pos1 != std::string::npos)
    {
        _httpType = httpurl.substr(0, pos1);
        pos1 += 3;
    }
    else {
        pos1 = 0;
    }
    auto pos2 = httpurl.find('/', pos1);
    if (pos2 != std::string::npos)
    {
        _host = httpurl.substr(pos1, pos2 - pos1);
        _path = httpurl.substr(pos2);

        pos1 = _path.find('?');
        if (pos1 != std::string::npos)
        {
            _args = _path.substr(pos1 + 1);
            _path = _path.substr(0, pos1);
        }
    }
    else {
        _host = httpurl.substr(pos1);
    }

    pos1 = _host.find(':');
    if (pos1 != std::string::npos)
    {
        _port = _host.substr(pos1 + 1);
        _host = _host.substr(0, pos1);
    }
}