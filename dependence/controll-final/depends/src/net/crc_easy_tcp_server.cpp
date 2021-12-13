#include "crc_easy_tcp_server.h"

CRCEasyTcpServer::CRCEasyTcpServer()
{
    _sock           = INVALID_SOCKET;
    _recvCount      = 0;
    _msgCount       = 0;
    _clientAccept   = 0;
    _clientJoin     = 0;
    _nSendBuffSize  = CRCConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
    _nRecvBuffSize  = CRCConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);
    _nMaxClient     = CRCConfig::Instance().getInt("nMaxClient", FD_SETSIZE);    
}

CRCEasyTcpServer::~CRCEasyTcpServer()
{
    Close();
}

SOCKET
CRCEasyTcpServer::InitSocket(int af)
{
    CRCNetWork::Init();
    if (INVALID_SOCKET != _sock)
    {
        CRCLog_Warring("initSocket close old socket<%d>...", (int)_sock);
        Close();
    }
    _address_family = af;
    _sock = socket(af, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == _sock)
    {
        CRCLog_PError("create socket failed...");
    }
    else {
        CRCNetWork::make_reuseaddr(_sock);
        CRCLog_Info("create socket<%d> success...", (int)_sock);
    }
    return _sock;
}

int
CRCEasyTcpServer::Bind(const char* ip, unsigned short port)
{
    int ret = SOCKET_ERROR;
    if (AF_INET == _address_family)
    {
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);//host to net unsigned short
#ifdef _WIN32
        if (ip) {
            _sin.sin_addr.S_un.S_addr = inet_addr(ip);
        }
        else {
            _sin.sin_addr.S_un.S_addr = INADDR_ANY;
        }
#else
        if (ip) {
            _sin.sin_addr.s_addr = inet_addr(ip);
        }
        else {
            _sin.sin_addr.s_addr = INADDR_ANY;
        }
#endif
        ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
    }
    else if (AF_INET6 == _address_family) {
        sockaddr_in6 _sin = {};
        _sin.sin6_family = AF_INET6;
        _sin.sin6_port = htons(port);
        if (ip)
        {
            inet_pton(AF_INET6, ip, &_sin.sin6_addr);
        }
        else {
            _sin.sin6_addr = in6addr_any;
        }
        ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
    }
    else {
        CRCLog_Error("bind port,_address_family<%d> failed...", _address_family);
        return ret;
    }

    if (SOCKET_ERROR == ret)
    {
        CRCLog_PError("bind port<%d> failed...", port);
    }
    else {
        CRCLog_Info("bind port<%d> success...", port);
    }
    return ret;
}

int
CRCEasyTcpServer::Listen(int n)
{
    int ret = listen(_sock, n);
    if (SOCKET_ERROR == ret){
        CRCLog_PError("listen socket<%d> failed...",_sock);
    } else {
        CRCLog_Info("listen port<%d> success...", _sock);
    }
    return ret;
}

SOCKET 
CRCEasyTcpServer::Accept()
{
    if (AF_INET == _address_family)
    {
        return Accept_IPv4();
    }
    else {
        return Accept_IPv6();
    }
}

void
CRCEasyTcpServer::AddClientToCRCWorkServer(CRCClient* pClient)
{
    //查找客户数量最少的 CRCWorkServer
    auto pMinServer = _workServers[0];
    for (auto pServer : _workServers)
    {
        if (pMinServer->getClientCount() > pServer->getClientCount()){
            pMinServer = pServer;
        }
    }
    //客户放入 CRCWorkServer
    pMinServer->addClient(pClient);
}

void 
CRCEasyTcpServer::Close()
{
    CRCLog_Info("EasyTcpServer.Close begin");

    _thread.Close();
    if (INVALID_SOCKET != _sock)
    {
        for (auto server : _workServers){
            delete server;
        }
        _workServers.clear();
        CRCNetWork::destorySocket(_sock);
        _sock = INVALID_SOCKET;
    }

    CRCLog_Info("EasyTcpServer.Close end");
}

void 
CRCEasyTcpServer::OnNetJoin(CRCClient* pClient)
{
    _clientJoin++;
    //CRCLog_Info("client<%d> join", pClient->sockfd());
}

void 
CRCEasyTcpServer::OnNetLeave(CRCClient* pClient)
{
    _clientAccept--;
    _clientJoin--;
    //CRCLog_Info("client<%d> leave", pClient->sockfd());
}

void 
CRCEasyTcpServer::OnNetMsg(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header)
{
    _msgCount++;
}

void 
CRCEasyTcpServer::OnNetRecv(CRCClient* pClient)
{
    _recvCount++;
}

void 
CRCEasyTcpServer::time4msg()
{
    auto t1 = _tTime.getElapsedSecond();
    if (t1 >= 10.0)
    {
        CRCLog_Info("thread<%d>,time<%lf>,socket<%d>,Accept<%d>,Join<%d>,recv<%d>,msg<%d>"
            , (int)_workServers.size()
            , t1, _sock
            , (int)_clientAccept, (int)_clientJoin
            , (int)_recvCount, (int)_msgCount);
        _recvCount = 0;
        _msgCount = 0;
        _tTime.update();
    }
}

SOCKET 
CRCEasyTcpServer::Accept_IPv6()
{
    sockaddr_in6 clientAddr = {};
    int nAddrLen = sizeof(clientAddr);
    SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
    cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
    cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
    if (INVALID_SOCKET == cSock)
    {
        CRCLog_PError("accept INVALID_SOCKET...");
    }
    else
    {
        //获取IP地址
        static char ip[INET6_ADDRSTRLEN] = {};
        inet_ntop(AF_INET6, &clientAddr.sin6_addr, ip, INET6_ADDRSTRLEN - 1);
        AcceptClient(cSock, ip);
    }
    return cSock;
}

SOCKET 
CRCEasyTcpServer::Accept_IPv4()
{
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(sockaddr_in);
    SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
    cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
    cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t *)&nAddrLen);
#endif
    if (INVALID_SOCKET == cSock)
    {
        CRCLog_PError("accept INVALID_SOCKET...");
    }
    else
    {
        //获取IP地址
        char* ip = inet_ntoa(clientAddr.sin_addr);
        AcceptClient(cSock, ip);
    }
    return cSock;
}

void 
CRCEasyTcpServer::AcceptClient(SOCKET cSock, char* ip)
{
    CRCNetWork::make_reuseaddr(cSock);
    CRCLog_Info("Accept_IP: %s, %d", ip, cSock);
    if (_clientAccept < _nMaxClient)
    {
        _clientAccept++;
        //将新客户端分配给客户数量最少的CRCServer
        auto c = makeClientObj(cSock);
        c->setIP(ip);
        AddClientToCRCWorkServer(c);
    }
    else {
        CRCNetWork::destorySocket(cSock);
        CRCLog_Warring("Accept to nMaxClient");
    }
}

CRCClient* 
CRCEasyTcpServer::makeClientObj(SOCKET cSock)
{
    return new CRCClient(cSock, _nSendBuffSize, _nRecvBuffSize);
}