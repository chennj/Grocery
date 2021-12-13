#include "crc_fdset.h"

CRCFDSet::CRCFDSet()
{

}

CRCFDSet::~CRCFDSet()
{
    destory();
}

void 
CRCFDSet::create(int MaxFds)
{
    int nSocketNum = MaxFds;
#ifdef _WIN32
    if (nSocketNum < 64)
        nSocketNum = 64;
    _nfdSize = sizeof(u_int) + (sizeof(SOCKET)*nSocketNum);
#else
    //用8KB存储65535个sockfd 8192byte*8 = 65536
    if (nSocketNum < 1024)
        nSocketNum = 1024;
    _nfdSize = nSocketNum / (8 * sizeof(char))+1;
#endif // _WIN32
    _pfdset = (fd_set *)new char[_nfdSize];
    memset(_pfdset, 0, _nfdSize);
    _MAX_SOCK_FD = nSocketNum;
}

void 
CRCFDSet::destory()
{
    if (_pfdset)
    {
        delete[] _pfdset;
        _pfdset = nullptr;
    }
}

void 
CRCFDSet::copy(CRCFDSet& set)
{
    memcpy(_pfdset, set.fdset(), set._nfdSize);
}