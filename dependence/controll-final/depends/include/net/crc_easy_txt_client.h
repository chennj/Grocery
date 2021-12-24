#ifndef _CRC_EASY_TXT_CLIENT_H_
#define _CRC_EASY_TXT_CLIENT_H_

#include "crc_tcp_client_mgr.h"
#include "crc_client_c_txt.h"

class CRCEasyWebSocketClient;

class CRCEasyTxtClient : public CRCIO::TcpClientMgr
{
public:
    explicit CRCEasyTxtClient();

public:
    //建立一个 CRCClientCTxt
    virtual CRCClient* makeClientObj(SOCKET cSock, int sendSize, int recvSize) override;
    //响应网络消息
    virtual void OnNetMsg(CRCDataHeader* header) override;

    //设置发送缓冲区大小
    void send_buff_size(int n);
    //设置接收缓冲区大小
    void recv_buff_size(int n);
    //发送字符串
    int  writeText(const char* pData, int len);
    //检查认证
    bool auth();
    //连接，并发送认证请求，修改连接状态为 join
    bool connect(int af, const char* ip, unsigned short port);
    //设置返回的源 client
    void setPairClient(CRCEasyWebSocketClient* client);

private:
    //客户端发送缓冲区大小
    int _nSendBuffSize = SEND_BUFF_SZIE;
    //客户端接收缓冲区大小
    int _nRecvBuffSize = RECV_BUFF_SZIE;
    //是否接到回复再发送下一条消息
    bool _sendback = false;
    //pair client
    CRCEasyWebSocketClient* _pPairClient;

public:
    //回调函数定义
    typedef std::function<void(CRCClientCTxt*)> EventCall;
    //留给外部使用的回调函数
    EventCall onmessage = nullptr;
};

#endif //!_CRC_EASY_TXT_CLIENT_H_