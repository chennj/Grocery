#include "crc_easy_tcp_client.h"

CRCEasyTcpClient::CRCEasyTcpClient()
{
    _isConnect = false;
}

CRCEasyTcpClient::~CRCEasyTcpClient()
{
    Close();
}

SOCKET 
CRCEasyTcpClient::InitSocket(int af,int sendSize, int recvSize)
{
    CRCNetWork::Init();

    if (_pClient)
    {
        CRCLog_Info("warning, initSocket close old socket<%d>...", (int)_pClient->sockfd());
        Close();
    }
    _address_family = af;
    SOCKET sock = socket(af, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sock)
    {
        CRCLog_PError("create socket failed...");
    }
    else {
        CRCNetWork::make_reuseaddr(sock);
        //CRCLog_Info("create socket<%d> success...", (int)sock);
        _pClient = makeClientObj(sock, sendSize, recvSize);
        OnInitSocket();
    }
    return sock;
}

int 
CRCEasyTcpClient::Connect(const char* ip, unsigned short port)
{
    if (!_pClient)
    {
        return SOCKET_ERROR;
    }
    int ret = SOCKET_ERROR;
    if (AF_INET == _address_family)//ipv4
    {
        // 2 连接服务器 connect
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
#ifdef _WIN32
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
        _sin.sin_addr.s_addr = inet_addr(ip);
#endif
        //CRCLog_Info("<socket=%d> connecting <%s:%d>...", (int)_pClient->sockfd(), ip, port);
        ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
    }
    else {//ipv6
        sockaddr_in6 _sin = {};
        _sin.sin6_scope_id = if_nametoindex(_scope_id_name.c_str());
        _sin.sin6_family = AF_INET6;
        _sin.sin6_port = htons(port);
        inet_pton(AF_INET6, ip, &_sin.sin6_addr);
        ret = connect(_pClient->sockfd(), (sockaddr*)&_sin, sizeof(sockaddr_in6));
    }

    if (SOCKET_ERROR == ret)
    {
        CRCLog_PError("<socket=%d> connect <%s:%d> failed...", (int)_pClient->sockfd(), ip, port);
    }
    else {
        _isConnect = true;
        OnConnect();
        //CRCLog_Info("<socket=%d> connect <%s:%d> success...", (int)_pClient->sockfd(), ip, port);
    }
    return ret;
}

void 
CRCEasyTcpClient::Close()
{
    if (_pClient)
    {
        delete _pClient;
        _pClient = nullptr;

        _isConnect = false;
        OnDisconnect();
    }
}

bool 
CRCEasyTcpClient::isRun()
{
    return _pClient && _isConnect;
}

int 
CRCEasyTcpClient::RecvData()
{
    if (isRun())
    {
        //接收客户端数据
        int nLen = _pClient->RecvData();
        if (nLen > 0)
        {
            DoMsg();
        }
        return nLen;
    }
    return 0;
}

void 
CRCEasyTcpClient::DoMsg()
{
    //循环 判断是否有消息需要处理
    while (_pClient->hasMsg())
    {
        //处理网络消息
        OnNetMsg(_pClient->front_msg());

        if (_pClient->isClose())
        {
            Close();
            return;
        }

        //移除消息队列（缓冲区）最前的一条数据
        _pClient->pop_front_msg();
    }
}

int 
CRCEasyTcpClient::SendData(CRCDataHeader* header)
{
    if (isRun())
        return _pClient->SendData(header);
    return SOCKET_ERROR;
}

int 
CRCEasyTcpClient::SendData(const char* pData, int len)
{
    if (isRun())
        return _pClient->SendData(pData, len);
    return SOCKET_ERROR;
}

void 
CRCEasyTcpClient::setScopeIdName(std::string scope_id_name)
{
    _scope_id_name = scope_id_name;
}

CRCClient* 
CRCEasyTcpClient::makeClientObj(SOCKET cSock, int sendSize, int recvSize)
{
    return new CRCClient(cSock, sendSize, recvSize);
}

void 
CRCEasyTcpClient::OnInitSocket() 
{
};

void 
CRCEasyTcpClient::OnConnect() 
{
};

void 
CRCEasyTcpClient::OnDisconnect() 
{
};