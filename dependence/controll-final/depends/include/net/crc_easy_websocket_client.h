#ifndef _CRC_EASY_WEBSOCKET_CLIENT_H_
#define _CRC_EASY_WEBSOCKET_CLIENT_H_

#include "crc_tcp_client_mgr.h"
#include "crc_client_c_websocket.h"

class CRCEasyWebSocketClient : public CRCIO::TcpClientMgr
{
public:
    CRCEasyWebSocketClient();
protected:
    virtual CRCClient* makeClientObj(SOCKET cSock, int sendSize, int recvSize) override;

    virtual void OnDisconnect() override;
public:
    virtual void OnNetMsg(CRCDataHeader* header) override;

    bool handshake();

    bool connect(const char* httpurl);

    int  hostname2ip(const char* hostname, const char* port);

    int  writeText(const char* pData, int len);

    void send_buff_size(int n);

    void recv_buff_size(int n);

private:
    void url2get(const char* host, const char* path, const char* args);

    bool connet2ip(int af, const char* ip, unsigned short port);

    void deatch_http_url(std::string httpurl);
    
private:
    std::string _httpType;
    std::string _host;
    std::string _port;
    std::string _path;
    std::string _args;
    ////
    std::string _cKey;
    //
    CRCClientCWebSocket* _pWSClient = nullptr;
    //客户端发送缓冲区大小
    int _nSendBuffSize = SEND_BUFF_SZIE;
    //客户端接收缓冲区大小
    int _nRecvBuffSize = RECV_BUFF_SZIE;
public:
    typedef std::function<void(CRCClientCWebSocket*)> EventCall;
    EventCall onopen = nullptr;
    EventCall onclose = nullptr;
    EventCall onmessage = nullptr;
    EventCall onerror = nullptr;
};

#endif //!_CRC_EASY_WEBSOCKET_CLIENT_H_