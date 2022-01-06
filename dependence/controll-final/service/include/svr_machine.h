#ifndef _SVR_MACHINE_H_
#define _SVR_MACHINE_H_

#include "crc_net_client_c.h"
#include "crc_easy_txt_client.h"
#include "crc_thread.h"
#include "svr_machine_midasbox.h"
#include <queue>
#include <mutex>

#define CMD_MAILBOX         "cs_machine_mailbox"
#define CMD_CDROM           "cs_machine_cdrom"
#define CMD_PRINTER         "cs_machine_printer"
#define CMD_QUERY           "cs_machine_query"

class MachineServer
{
private:
    //连接总控服务区的客户端
    //负责服务的注册，空闲心跳，自动重连
    CRCNetClientC           _csCtrl;
    //连接设备服务器的客户端
    //负责服务的注册，空闲心跳，自动重连
    CRCEasyTxtClient        _csMachine;
    //连接设备服务器的线程
    CRCThread               _thread;
    //任务缓冲队列
    std::queue<CRCJson*>    _task_queue;
    //任务缓冲队列锁
    std::mutex              _task_queue_mtx;
    //本地任务队列
    //优先级高于客户端发过来的任务队列即（_task_queue）
    //一般用于获取at91或以后的stm32的设备信息，在第一次连接、断线重连、异常发生
    //（如，开门等）
    std::queue<CRCJson*>    _local_task_queue;
    //来之at91的 MidasBox
    //记录着所有设备信息
    MidasBox                _midasBox;
    //是否需要重新获取 MidasBox
    bool                    _isNeedGetMidasBox = true;
public:
    virtual ~MachineServer();

public:
    void Init();

    void Run();

    void Close();

    //分析客户端的命令是否合法
    virtual bool ParseCmd(const CRCJson & json, std::string & cmd) const;

    //处理从at91返回的消息
    virtual bool OnProcess4Equipment(CRCJson * pMsg, std::string& str4Eqpt);

    void MachineLoop(CRCThread* pThread);

    inline CRCEasyTxtClient& machine(){return _csMachine;}

private:
    void onopen_csCtrl(CRCNetClientC* client, CRCJson& msg);

    void cs_machine_mailbox(CRCNetClientC* client, CRCJson& msg);
};

#endif