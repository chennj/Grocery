#include "crc_buffer.h"

CRCBuffer::CRCBuffer(int nSize)
{
    _nSize = nSize;
    _pBuff = new char[_nSize];
}

CRCBuffer::~CRCBuffer()
{
    if (_pBuff)
    {
        delete[] _pBuff;
        _pBuff = nullptr;
    }
}

bool
CRCBuffer::push(const char* pData, int nLen)
{
    ////写入大量数据不一定要放到内存中
    ////也可以存储到数据库或者磁盘等存储器中
    //if (_nLast + nLen > _nSize)
    //{
    //	//需要写入的数据大于可用空间
    //	int n = (_nLast + nLen) - _nSize;
    //	//拓展BUFF
    //	if (n < 8192)
    //		n = 8192;
    //	char* buff = new char[_nSize+n];
    //	memcpy(buff, _pBuff, _nLast);
    //	delete[] _pBuff;
    //	_pBuff = buff;
    //}

    if (_nLast + nLen <= _nSize)
    {
        //将要发送的数据 拷贝到发送缓冲区尾部
        memcpy(_pBuff + _nLast, pData, nLen);
        //计算数据尾部位置
        _nLast += nLen;

        if (_nLast == SEND_BUFF_SZIE)
        {
            CRCLog_Warring("recv buff full nSize<%d> nLast<%d> ", _nSize, _nLast);
            ++_fullCount;
        }

        return true;
    }
    else {
        CRCLog_Warring("recv buff full nSize<%d> nLast<%d> ", _nSize, _nLast);
        ++_fullCount;
    }

    return false;
}

void
CRCBuffer::pop(int nLen)
{
    int n = _nLast - nLen;
    if (n > 0)
    {
        memcpy(_pBuff, _pBuff + nLen, n);
    }
    _nLast = n;
    if (_fullCount > 0)
        --_fullCount;
}

int
CRCBuffer::write2socket(SOCKET sockfd)
{
    int ret = 0;
    //缓冲区有数据
    if (_nLast > 0 && INVALID_SOCKET != sockfd)
    {
        //发送数据
        ret = send(sockfd, _pBuff, _nLast, 0);
        if (ret <= 0)
        {
            CRCLog_PError("write2socket1:sockfd<%d> nSize<%d> nLast<%d> ret<%d>", sockfd, _nSize, _nLast, ret);
            return SOCKET_ERROR;
        }
        if (ret == _nLast)
        {//_nLast=2000 实际发送ret=2000
            //数据尾部位置清零
            _nLast = 0;
        }
        else {
            //_nLast=2000 实际发送ret=1000
            //CRCLog_Info("write2socket2:sockfd<%d> nSize<%d> nLast<%d> ret<%d>", sockfd, _nSize, _nLast, ret);
            _nLast -= ret;
            memcpy(_pBuff, _pBuff + ret, _nLast);
        }
        _fullCount = 0;
    }
    return ret;
}

int
CRCBuffer::read4socket(SOCKET sockfd)
{
    if (_nSize - _nLast > 0)
    {
        //接收客户端数据
        char* szRecv = _pBuff + _nLast;
        int nLen = (int)recv(sockfd, szRecv, _nSize - _nLast, 0);
        if (nLen <= 0)
        {
            CRCLog_PError("read4socket:sockfd<%d> nSize<%d> nLast<%d> nLen<%d>", sockfd, _nSize, _nLast, nLen);
            return SOCKET_ERROR;
        }
        //消息缓冲区的数据尾部位置后移
        _nLast += nLen;
        return nLen;
    }
    else 
    {
        CRCLog_Warring("read4socket _nSize<%d> - _nLast<%d> < 0", _nSize, _nLast);
    }
    return 0;
}

bool
CRCBuffer::hasMsg()
{
    //判断消息缓冲区的数据长度大于消息头CRCDataHeader长度
    if (_nLast >= sizeof(CRCDataHeader))
    {
        //这时就可以知道当前消息的长度
        CRCDataHeader* header = (CRCDataHeader*)_pBuff;
        //判断消息缓冲区的数据长度大于消息长度
        return _nLast >= header->dataLength;
    }
    return false;
}