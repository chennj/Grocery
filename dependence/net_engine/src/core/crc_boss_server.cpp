#include "../../include/crc_boss_server.h"

CRCBossServer::CRCBossServer()
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

CRCBossServer::~CRCBossServer()
{
    Close();
}

SOCKET
CRCBossServer::InitSocket()
{
    CRCNetWork::Init();
    if (INVALID_SOCKET != _sock)
    {
        CRCLog_Warring("initSocket close old socket<%d>...", (int)_sock);
        Close();
    }
    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
CRCBossServer::Bind(const char* ip, unsigned short port)
{
    // 2 bind 绑定用于接受客户端连接的网络端口
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(port);//host to net unsigned short

#ifdef _WIN32
    if (ip){
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
    int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin));
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
CRCBossServer::Listen(int n)
{
    // 3 listen 监听网络端口
    int ret = listen(_sock, n);
    if (SOCKET_ERROR == ret)
    {
        CRCLog_PError("listen socket<%d> failed...",_sock);
    }
    else {
        CRCLog_Info("listen port<%d> success...", _sock);
    }
    return ret;    
}

SOCKET
CRCBossServer::Accept()
{
    // 4 accept 等待接受客户端连接
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
        if (_clientAccept < _nMaxClient)
        {
            _clientAccept++;
            CRCNetWork::make_reuseaddr(cSock);
            //将新客户端分配给客户数量最少的cellServer
            addClientToWorkServer(new CRCChannel(cSock, _nSendBuffSize, _nRecvBuffSize));
            //获取IP地址 inet_ntoa(clientAddr.sin_addr)
        }
        else {
            //获取IP地址 inet_ntoa(clientAddr.sin_addr)
            CRCNetWork::destorySocket(cSock);
            CRCLog_Warring("Accept to nMaxClient");
        }
    }
    return cSock;    
}

void
CRCBossServer::addClientToWorkServer(CRCChannel* pClient)
{
    //查找客户数量最少的CRCWorkServer消息处理对象
    auto pMinServer = _workServers[0];
    for(auto pServer : _workServers)
    {
        if (pMinServer->getClientCount() > pServer->getClientCount())
        {
            pMinServer = pServer;
        }
    }
    pMinServer->addClient(pClient);
}

void
CRCBossServer::Close()
{
    CRCLog_Info("CRCBossServer.Close begin");
    _thread.Close();
    if (_sock != INVALID_SOCKET)
    {
        for (auto s : _workServers)
        {
            delete s;
        }
        _workServers.clear();
        //关闭套节字socket
        CRCNetWork::destorySocket(_sock);
        _sock = INVALID_SOCKET;
    }
    CRCLog_Info("CRCBossServer.Close end");    
}

void
CRCBossServer::OnNetJoin(CRCChannel* pClient)
{
    _clientJoin++;    
    //CRCLog_Info("client<%d> join", pClient->sockfd());
}

void
CRCBossServer::OnNetLeave(CRCChannel* pClient)
{
    _clientAccept--;
    _clientJoin--;
    //CRCLog_Info("client<%d> leave", pClient->sockfd());
}

void
CRCBossServer::OnNetMsg(CRCWorkServer* pServer, CRCChannel* pClient, CRCDataHeader* header)
{
    _msgCount++;
}

void
CRCBossServer::OnNetRecv(CRCChannel* pClient)
{
    _recvCount++;
}

void
CRCBossServer::time4msg()
{
    auto t1 = _tTime.getElapsedSecond();
    if (t1 >= 1.0)
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