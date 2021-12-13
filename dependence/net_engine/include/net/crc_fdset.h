#ifndef _CRC_FDSET_H_
#define _CRC_FDSET_H_

#include "crc_base.h"

class CRCFDSet
{
public:
	CRCFDSet();

	~CRCFDSet();

	//Linux下表示socket fd的最大值
	//Windows下表示socket fd的数量
	void create(int MaxFds);

	void destory();

	void copy(CRCFDSet& set);
	
	inline void add(SOCKET s)
	{
#ifdef _WIN32
		FD_SET(s, _pfdset);
#else
		if(s < _MAX_SOCK_FD)
		{
			FD_SET(s, _pfdset);
		}else{
			CRCLog_Error("CRCFDSet::add sock<%d>, CRC_MAX_FD<%d>",(int)s,_MAX_SOCK_FD);
		}
#endif // _WIN32
	}

	inline void del(SOCKET s)
	{
		FD_CLR(s, _pfdset);
	}

	inline void zero()
	{
#ifdef _WIN32
		FD_ZERO(_pfdset);
#else
		memset(_pfdset, 0, _nfdSize);
#endif // _WIN32
	}

	inline bool has(SOCKET s)
	{
		return FD_ISSET(s, _pfdset);
	}

	inline fd_set* fdset()
	{
		return _pfdset;
	}

private:
	fd_set * _pfdset = nullptr;
	size_t _nfdSize = 0;
	int _MAX_SOCK_FD =0;
};


#endif // !_CRC_FDSET_H_
