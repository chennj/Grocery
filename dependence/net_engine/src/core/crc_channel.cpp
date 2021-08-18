#include "../../include/crc_channel.h"

CRCChannel::CRCChannel(SOCKET sockfd, int sendSize, int recvSize):
    _sockfd(sockfd),
    _sendBuff(sendSize),
    _recvBuff(recvSize)
{
    static int n = 1;
    id = n++;

    resetDTHeart();
    resetDTSend();
}

CRCChannel::~CRCChannel()
{
    destory();
}

void
CRCChannel::destory()
{
    if (INVALID_SOCKET != _sockfd)
    {
        //CRCLog_Info("CRCChannel::destory[sId=%d id=%d socket=%d]", serverId, id, (int)_sockfd);
        CRCNetWork::destorySocket(_sockfd);
        _sockfd = INVALID_SOCKET;
    }
}

int
CRCChannel::RecvData()
{
    return _recvBuff.read4socket(_sockfd);
}

bool
CRCChannel::hasMsg()
{
    return _recvBuff.hasMsg();
}

CRCDataHeader*
CRCChannel::front_msg()
{
    return (CRCDataHeader*)_recvBuff.data();
}

void
CRCChannel::pop_front_msg()
{
    if(hasMsg()){
        _recvBuff.pop(front_msg()->dataLength);
    }
}

bool
CRCChannel::needWrite()
{
    return _sendBuff.needWrite();
}

int
CRCChannel::SendDataReal()
{
    resetDTSend();
	return _sendBuff.write2socket(_sockfd);
}

int 
CRCChannel::SendData(CRCDataHeader* header)
{
    return SendData((const char*)header, header->dataLength);
}

int
CRCChannel::SendData(const char* pData, int len)
{
    if (_sendBuff.push(pData, len))
    {
        return len;
    }
    return SOCKET_ERROR;    
}

bool
CRCChannel::checkHeart(time_t dt)
{
    _dtHeart += dt;
    if (_dtHeart >= CLIENT_HREAT_DEAD_TIME)
    {
        CRCLog_Info("checkHeart dead:s=%d,time=%ld",_sockfd, _dtHeart);
        return true;
    }
    return false;    
}

bool
CRCChannel::checkSend(time_t dt)
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