#ifndef _CRC_NET_WORK_H_
#define _CRC_NET_WORK_H_
// file: crc_net_work.h

#include"crc_common.h"
#include"crc_log.h"
#ifndef _WIN32
#include<fcntl.h>
#include<stdlib.h>
#endif // !_WIN32

class CRCNetWork
{
private:
    CRCNetWork();
    ~CRCNetWork();

public:
	static void Init();
    static int make_nonblocking(SOCKET fd);
    static int make_reuseaddr(SOCKET fd);
    static int make_nodelay(SOCKET fd);
    static int destorySocket(SOCKET sockfd);
};

#endif