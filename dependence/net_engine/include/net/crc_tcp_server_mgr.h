#ifndef _CRC_TCP_SERVER_MGR_H_
#define _CRC_TCP_SERVER_MGR_H_

#if _WIN32
	#include "crc_easy_iocp_server.h"
#elif __linux__
	#include "crc_easy_epoll_server.h"
#else
	#include "crc_easy_selct_server.h"
#endif

namespace CRCIO {
#if _WIN32
		typedef CRCEasyIocpServer TcpServerMgr;
#elif __linux__
		typedef CRCEasyEpollServer TcpServerMgr;
#else
		typedef CRCEasySelectServer TcpServerMgr;
#endif
}

#endif //!_CRC_TCP_SERVER_MGR_H_