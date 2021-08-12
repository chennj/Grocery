#ifndef _CRC_TCPSERVER_H_
#define _CRC_TCPSERVER_H_
// file crc_tcpserver.h

#include "crc_common.h"
#include "crc_channel.h"
#include "crc_work_server.h"
#include "crc_i_net_event.h"
#include "crc_net_work.h"
#include "crc_config.h"

#include <thread>
#include <mutex>
#include <atomic>

class CRCTcpServer : public CRCINetEvent
{

};

#endif