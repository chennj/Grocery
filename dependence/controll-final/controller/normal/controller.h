/**
 * author:chenningjiang
 * 普通控制器
 * 主要负责刻录打印的任务调度以及其他的业务逻辑
 * 
 * */
#ifndef _NORMAL_CONTROLLER_H_
#define _NORMAL_CONTROLLER_H_

#include "crc_net_client_c.h"
#include "crc_thread_pool.h"

class NlController
{
private:
    //线程池
    CRCThreadPool   m_thread_pool;
    //连接网关服务器
    CRCNetClientC   m_csGate;
public:
    void Init();

    void Run();

    void Close();

private:
    void onopen_csGate(CRCNetClientC* client, CRCJson& msg);

    bool wait_for(std::condition_variable& condition, const std::string& msg, uint32_t timeout = 60 * 1000);

    int  ctl_nl_burn_iso_file(CRCNetClientC* client, CRCJson& msg);
    
};

#endif // _NORMAL_CONTROLLER_H_