#include "crc_client.h"

CRCClient::CRCClient(SOCKET sockfd, int sendSize, int recvSize)
:
_sockfd(sockfd),
_sendBuff(sendSize),
_recvBuff(recvSize)
{
    static int n = 1;
    id = n++;
    //_sockfd = sockfd;

    resetDTHeart();
    resetDTSend();

    //CRCLog_Info("Channel Init: sockfd<%d> sendBuff Size<%d> recvBuff Size<%d>", _sockfd, _sendBuff, _recvBuff);
}

CRCClient::~CRCClient()
{
    //CRCLog_Info("~CRCClient[sId=%d id=%d socket=%d]", serverId, id, (int)_sockfd);
    destory();
}

void
CRCClient::destory()
{
    if (INVALID_SOCKET != _sockfd)
    {
        //CRCLog_Info("CRCClient::destory[sId=%d id=%d socket=%d]", serverId, id, (int)_sockfd);
        CRCNetWork::destorySocket(_sockfd);
        _sockfd = INVALID_SOCKET;
    }
}

int
CRCClient::RecvData()
{
    return _recvBuff.read4socket(_sockfd);
}

bool
CRCClient::hasMsg()
{
    return _recvBuff.hasMsg();
}

CRCDataHeader*
CRCClient::front_msg()
{
    return (CRCDataHeader*)_recvBuff.data();
}

void
CRCClient::pop_front_msg()
{
    if(hasMsg())
        _recvBuff.pop(front_msg()->dataLength);
}

bool
CRCClient::needWrite()
{
    return _sendBuff.needWrite();
}

void 
CRCClient::onSendComplete(){}

int
CRCClient::SendDataReal()
{
    resetDTSend();
    int ret = _sendBuff.write2socket(_sockfd);
    //判断所有数据发送完成
    if (_sendBuff.dataLen() == 0)
    {
        onSendComplete();
    }
    return ret;
}

int
CRCClient::SendData(CRCDataHeader* header)
{
    return SendData((const char*)header, header->dataLength);
}

int
CRCClient::SendData(const char* pData, int len)
{
    if (_sendBuff.push(pData, len))
    {
        return len;
    }
    return SOCKET_ERROR;
}

bool
CRCClient::checkHeart(time_t dt)
{
    if (isClose()){
        return true;
    }

    _dtHeart += dt;
    if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
    {
        CRCLog_Info("checkHeart dead:s=%d,time=%ld",_sockfd, _dtHeart);
        return true;
    }
    return false;
}

bool
CRCClient::checkSend(time_t dt)
{
    _dtSend += dt;
    if (_dtSend >= CLIENT_SEND_BUFF_TIME)
    {
        CRCLog_Info("checkSend:s=%d,time=%d", _sockfd, _dtSend);
        //立即将发送缓冲区的数据发送出去
        SendDataReal();
        //重置发送计时
        resetDTSend();
        return true;
    }
    return false;
}

void 
CRCClient::setIP(char* ip)
{
    if(ip){
        strncpy(_ip, ip, INET6_ADDRSTRLEN);
    }
}

void
CRCClient::onClose()
{
    CRCLog_Info("sockfd<%d> onClose", _sockfd);
    state(clientState_close);
}