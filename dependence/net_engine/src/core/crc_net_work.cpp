#include "../../include/crc_net_work.h"

CRCNetWork::CRCNetWork()
{
#ifdef _WIN32
    //启动Windows socket 2.x环境
    WORD ver = MAKEWORD(2, 2);
    WSADATA dat;
    WSAStartup(ver, &dat);
#endif

#ifndef _WIN32
    //if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    //	return (1);
    //忽略异常信号，默认情况会导致进程终止
    signal(SIGPIPE, SIG_IGN);
#endif
}

CRCNetWork::~CRCNetWork()
{
#ifdef _WIN32
    //清除Windows socket环境
    WSACleanup();
#endif    
}

void
CRCNetWork::Init()
{
    static CRCNetWork obj;
}

int 
CRCNetWork::make_nonblocking(SOCKET fd)
{
#ifdef _WIN32
    {
        unsigned long nonblocking = 1;
        if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
            CRCLog_Warring("fcntl(%d, F_GETFL)", (int)fd);
            return -1;
        }
    }
#else
    {
        int flags;
        if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) {
            CRCLog_Warring("fcntl(%d, F_GETFL)", fd);
            return -1;
        }
        if (!(flags & O_NONBLOCK)) {
            if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
                CRCLog_Warring("fcntl(%d, F_SETFL)", fd);
                return -1;
            }
        }
    }
#endif
    return 0;    
}

int
CRCNetWork::make_reuseaddr(SOCKET fd)
{
    int flag = 1;
    if (SOCKET_ERROR == setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&flag, sizeof(flag))) {
        CRCLog_Warring("setsockopt socket<%d> SO_REUSEADDR failed",(int)fd);
        return SOCKET_ERROR;
    }
    return 0;    
}