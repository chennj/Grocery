#include "crc_client_c_txt.h"

CRCClientCTxt::CRCClientCTxt(SOCKET sockfd, int sendSize, int recvSize) :
    CRCClient(sockfd, sendSize,recvSize)
{
}

bool 
CRCClientCTxt::hasMsg()
{
    //完整的消息一定超过3字节
    if (_recvBuff.dataLen() < 3){
        return false;
    }

    int ret = checkTxtResponse();
    if (ret < 0)
        CRCLog_Info("CRCClientCTxt: checkTxtResponse error msg.");
    return ret > 0;    
}

int 
CRCClientCTxt::checkTxtResponse()
{
     CRCLog_Info("CRCClientCTxt::checkTxtResponse() <%s>", _recvBuff.data());

    //查找http消息结束标记
    char* temp = strstr(_recvBuff.data(), "\n");

    //未找到表示消息还不完整
    if (!temp){
        return 0;
    }

    //偏移到消息结束位置
    //1=strlen("\n")
    temp += 1;
    //计算响应消息长度
    _txtLen = temp - _recvBuff.data();
    return 1;
}

void 
CRCClientCTxt::pop_front_msg()
{
    if (_txtLen > 0)
    {
        _recvBuff.pop(_txtLen);
        _txtLen = 0;
    }    
}

bool 
CRCClientCTxt::getResponseInfo()
{
    //判断是否已经收到了完整消息
    if (_txtLen <= 0){
        return false;
    }
 
    char* pp = _recvBuff.data();
    pp[_txtLen-1] = '\0';

    CRCLog_Info("CRCClientCTxt::getResponseInfo <%s>", _recvBuff.data());
    _content =_recvBuff.data();
}

void 
CRCClientCTxt::ping()
{
    SendData("PING", strlen("PING"));
}

void 
CRCClientCTxt::pong()
{
    SendData("PONG", strlen("PONG"));
}