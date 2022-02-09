/**
 * 
 * author:  chenningjiang
 * desc:    使用select模型的接收服务端
 * 
 * */
#ifndef _CRC_EASY_SELECT_SERVER_H_
#define _CRC_EASY_SELECT_SERVER_H_

#include "crc_easy_tcp_server.h"
#include "crc_work_select_server.h"
#include "crc_fdset.h"

class CRCEasySelectServer : public CRCEasyTcpServer
{
public:
    virtual ~CRCEasySelectServer();
public:
    //启动select模型的CRCWorkServer服务器
    void Start(int nCRCWorkServer);

protected:
    //处理网络消息
    void OnRun(CRCThread* pThread) override;
};

#endif  //!_CRC_EASY_SELECT_SERVER_H_