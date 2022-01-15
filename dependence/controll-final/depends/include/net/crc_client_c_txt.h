#ifndef _CRC_CLIENT_C_TXT_H_
#define _CRC_CLIENT_C_TXT_H_

#include "crc_client.h"

class CRCClientCTxt : public CRCClient
{
public:
    explicit CRCClientCTxt() = delete;
    explicit CRCClientCTxt(SOCKET sockfd = INVALID_SOCKET, int sendSize = SEND_BUFF_SZIE, int recvSize = RECV_BUFF_SZIE);

public:
    //是否有新的消息进来
    virtual bool hasMsg() override;

    //弹出缓冲区第一个消息
    virtual void pop_front_msg() override;

    // 0 消息不完整 继续等待消息
    // -1 不支持的消息类型
    // -2 异常消息
    int  checkTxtResponse();

    //确定收到完整消息的时候才能调用
    bool getResponseInfo();

    //设置已经获得认证
    //用于判断收到的消息是在认证前，还是在认证后
    void setAuth(bool b);

    //获取接收到的原始数据
    char * getRecvData();

    //获取接收到的数据长度
    int  getRecvLen();

    //是否认证
    inline bool getAuth(){return _isAuthed;}

    //获取内容
    inline std::string& getContent(){return _content;}

    void ping();
    void pong();
public:
    int  _txtLen    = 0;
    bool _isAuthed  = false;
    std::string _content;
};

#endif //!_CRC_CLIENT_C_TXT_H_