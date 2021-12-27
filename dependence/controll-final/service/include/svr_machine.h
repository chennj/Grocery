#ifndef _SVR_MACHINE_H_
#define _SVR_MACHINE_H_

#include "crc_net_client_c.h"
#include "crc_easy_txt_client.h"
#include "crc_thread.h"
#include <queue>
#include <mutex>

class MachineServer
{
private:
    //连接总控服务区的客户端
    //负责服务的注册，空闲心跳，自动重连
    CRCNetClientC       _csCtrl;
    //连接设备服务器的客户端
    //负责服务的注册，空闲心跳，自动重连
    CRCEasyTxtClient    _csMachine;
    //连接设备服务器的线程
    CRCThread           _thread;
    //任务缓冲队列
    std::queue<CRCJson> _task_queue;
    //任务缓冲队列锁
    std::mutex          _task_queue_mtx;

public:
    void Init();

    void Run();

    void Close();

    void MachineLoop(CRCThread* pThread);

    inline CRCEasyTxtClient& machine(){return _csMachine;}

private:
    void onopen_csCtrl(CRCNetClientC* client, CRCJson& msg);

    void cs_machine_mailbox(CRCNetClientC* client, CRCJson& msg);
};

#endif