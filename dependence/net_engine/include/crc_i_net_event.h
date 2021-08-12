#ifndef _CRC_I_NET_EVENT_H_
#define _CRC_I_NET_EVENT_H_
// file: crc_i_net_event.h

#include "crc_common.h"
#include "crc_channel.h"

class CRCWorkServer;

//网络事件接口
class CRCINetEvent
{
public:
	//纯虚函数
	//客户端加入事件
	virtual void OnNetJoin(CRCChannel* pChannel) = 0;
	//客户端离开事件
	virtual void OnNetLeave(CRCChannel* pChannel) = 0;
	//客户端消息事件
	virtual void OnNetMsg(CRCWorkServer* pServer, CRCChannel* pChannel, CRCDataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv(CRCChannel* pChannel) = 0;
};

#endif