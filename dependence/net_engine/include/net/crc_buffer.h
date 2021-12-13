#ifndef _CRC_BUFFER_H_
#define _CRC_BUFFER_H_

#include "crc_base.h"

#ifdef CRC_USE_IOCP
#include "crc_iocp.h"
#endif

class CRCBuffer
{
public:
	CRCBuffer(int nSize = 8192);

	~CRCBuffer();

	inline char* data()
	{
		return _pBuff;
	}

	inline bool needWrite()
	{
		return _nLast > 0;
	}

	inline int dataLen()
	{
		return _nLast;
	}

	inline int buffSize()
	{
		return _nSize;
	}

	inline bool canWrite(int size)
	{
		return size < _nSize - _nLast;
	}

	bool push(const char* pData, int nLen);

	void pop(int nLen);

	int  write2socket(SOCKET sockfd);

	int  read4socket(SOCKET sockfd);

	bool hasMsg();

#ifdef CRC_USE_IOCP
	IO_DATA_BASE* makeRecvIoData(SOCKET sockfd)
	{
		int nLen = _nSize - _nLast;
		if (nLen > 0)
		{
			_ioData.wsabuff.buf = _pBuff + _nLast;
			_ioData.wsabuff.len = nLen;
			_ioData.sockfd = sockfd;
			return &_ioData;
		}
		return nullptr;
	}

	IO_DATA_BASE* makeSendIoData(SOCKET sockfd)
	{
		if (_nLast > 0)
		{
			_ioData.wsabuff.buf = _pBuff;
			_ioData.wsabuff.len = _nLast;
			_ioData.sockfd = sockfd;
			return &_ioData;
		}
		return nullptr;
	}

	bool read4iocp(int nRecv)
	{
		if (nRecv > 0 && _nSize - _nLast >= nRecv)
		{
			_nLast += nRecv;
			return true;
		}
		CRCLog_Error("read4iocp:sockfd<%d> nSize<%d> nLast<%d> nRecv<%d>", _ioData.sockfd, _nSize, _nLast, nRecv);
		return false;
	}

	bool write2iocp(int nSend)
	{
		if (_nLast < nSend)
		{
			CRCLog_Error("write2iocp:sockfd<%d> nSize<%d> nLast<%d> nSend<%d>", _ioData.sockfd, _nSize, _nLast, nSend);
			return false;
		}
		if (_nLast == nSend)
		{//_nLast=2000 实际发送nSend=2000
		 //数据尾部位置清零
			_nLast = 0;
		}
		else {
			//_nLast=2000 实际发送ret=1000
			_nLast -= nSend;
			memcpy(_pBuff, _pBuff + nSend, _nLast);
		}
		_fullCount = 0;
		return true;
	}
#endif // CRC_USE_IOCP
private:
	//第二缓冲区 发送缓冲区
	char* _pBuff = nullptr;
	//可以用链表或队列来管理缓冲数据块
	//list<char*> _pBuffList;
	//缓冲区的数据尾部位置，已有数据长度
	int _nLast = 0;
	//缓冲区总的空间大小，字节长度
	int _nSize = 0;
	//缓冲区写满次数计数
	int _fullCount = 0;
#ifdef CRC_USE_IOCP
	IO_DATA_BASE _ioData = {};
#endif // CRC_USE_IOCP
};

#endif // !_CRC_BUFFER_H_
