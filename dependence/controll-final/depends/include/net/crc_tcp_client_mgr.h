#ifndef _CRC_TCP_CLIENT_MGR_H_
#define _CRC_TCP_CLIENT_MGR_H_

#if _WIN32
	#include "crc_easy_iocp_client.h"
#elif __linux__
	#include "crc_easy_epoll_client.h"
#else
	#include "crc_easy_select_client.h"
#endif

namespace CRCIO{
#if _WIN32
		typedef CRCEasyIOCPClient TcpClientMgr;
#elif __linux__
		typedef CRCEasyEpollClient TcpClientMgr;
#else
		typedef CRCEasySelectClient TcpClientMgr;
#endif
}
#endif // !_CRC_TCP_CLIENT_MGR_H_