/**
 * 
 * author:  chenningjiang
 * desc:    网络事件接口
 * 
 * */
#ifndef _CRC_I_NET_EVENT_H_
#define _CRC_I_NET_EVENT_H_

#include "crc_base.h"
#include "crc_client.h"

//工作线程
class CRCWorkServer;

//网络事件接口
class CRCINetEvent
{
public:
	//纯虚函数
	//客户端加入事件
	virtual void OnNetJoin 	(CRCClient* pClient) = 0;
	//客户端离开事件
	virtual void OnNetLeave	(CRCClient* pClient) = 0;
	//客户端消息事件
	virtual void OnNetMsg 	(CRCWorkServer* pServer, CRCClient* pClient, CRCDataHeader* header) = 0;
	//recv事件
	virtual void OnNetRecv	(CRCClient* pClient) = 0;
private:

};

#endif // !_CRC_I_NET_EVENT_H_