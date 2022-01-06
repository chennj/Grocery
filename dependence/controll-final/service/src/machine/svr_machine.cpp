#include "svr_machine.h"

MachineServer::~MachineServer()
{
    while(!_task_queue.empty()){
        delete _task_queue.front();
        _task_queue.pop();
    }
}

void 
MachineServer::Init()
{
    //启动时获取设备信息的任务加入本地队列，确保是第一个执行的任务
    CRCJson * pMsg = new CRCJson();
    pMsg->Add("cmd","GMAP");
    _local_task_queue.push(pMsg);

    //连接总控,注册服务
    _csCtrl.set_groupid("0001");

    _csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    _csCtrl.reg_msg_call("onopen", std::bind(&MachineServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    _csCtrl.reg_msg_call("cs_machine_mailbox", std::bind(&MachineServer::cs_machine_mailbox, this, std::placeholders::_1, std::placeholders::_2));

    //设置处理来自设备的信息的回调处理函数
    _csMachine.onmessage = [this](CRCClientCTxt* pTxtClient){

        std::string& ss = pTxtClient->getContent();
        
        CRCJson* pmsg = nullptr;
        if (!_task_queue.empty())
        {
            std::lock_guard<std::mutex> lock(_task_queue_mtx);
            pmsg = _task_queue.front();
            _task_queue.pop();
        }

        CRCLog_Info("_csMachine.onmessage: recv %s; msg %s",ss.c_str(), pmsg->ToString().c_str());

        OnProcess4Equipment(pmsg, ss);

        delete pmsg;
    };

    //1. 连接设备，断线重连
    //2. 轮询任务列表
    //3. 分析任务，如果合法就向设备发送命令，否则直接返回客户端
    _thread.Start(
        //onCreate
        nullptr,
        //onRun
        [this](CRCThread* pThread) {
            MachineLoop(pThread);
        },
        //onDestory
        nullptr
    );
}

void 
MachineServer::Run()
{
     _csCtrl.run(1);
}

void 
MachineServer::Close()
{
    _csCtrl.close();
}

void 
MachineServer::MachineLoop(CRCThread* pThread)
{
    int s_size = CRCConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
    int r_size = CRCConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

    _csMachine.send_buff_size(s_size);
    _csMachine.recv_buff_size(r_size);        

    while(pThread->isRun())
    {

        if (_csMachine.isRun())
        {
            _csMachine.OnRun();
            //如果有任务
            if (!_task_queue.empty())
            {
                CRCJson* pmsg = _task_queue.front();
                {
                    //不做写动作，不用加锁
                    //std::lock_guard<std::mutex> lock(_task_queue_mtx);
                    //msg = _task_queue.front();
                }

                if ((*pmsg)("status").compare("ready") != 0){
                    continue;
                }

                std::string cmd;
                if (!ParseCmd((*pmsg),cmd)){
                    {
                        std::lock_guard<std::mutex> lock(_task_queue_mtx);
                        _task_queue.pop();
                    }
                    CRCJson ret;
                    ret.Add("data", "UNSUPPORTED command");
                    _csCtrl.response(*pmsg, ret);
                    delete pmsg;
                    continue;
                }
                
                if (SOCKET_ERROR != _csMachine.writeText(cmd.c_str(), cmd.length())){
                    pmsg->Replace("status", "sent");
                } else {       
                    CRCThread::Sleep(1000);
                }
                
            } else {
                CRCThread::Sleep(1);
            }

            continue;
            
        }

        if (_csMachine.connect(AF_INET,"111.111.1.100", 2020))
        {
            CRCLog_Info("MachineLoop::connect machine success.");
            continue;
        }

        CRCThread::Sleep(1000);
    }
}

void 
MachineServer::onopen_csCtrl(CRCNetClientC* client, CRCJson& msg)
{
    CRCJson json;
    json.Add("type",    "MachineServer");
    json.Add("name",    "MachineServer");
    json.Add("sskey",   "ssmm00@123456");
    json.AddEmptySubArray("apis");
    json["apis"].Add("cs_machine_cdrom");
    json["apis"].Add("cs_machine_mailbox");
    json["apis"].Add("cs_machine_printer");
    json["apis"].Add("cs_machine_query");

    client->request("ss_reg_api", json, [](CRCNetClientC* client, CRCJson& msg) {
        CRCLog_Info("MachineServer::ss_reg_api return: %s", msg("data").c_str());
    });
}

void 
MachineServer::cs_machine_mailbox(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("MachineServer::cs_machine_mailbox msg: %s", msg.ToString().c_str());

    CRCJson ret;
    if (!_thread.isRun())
    {
        ret.Add("data", "machine client is offline");
        client->response(msg, ret);
        return;
    }

    if (!_csMachine.isRun()){
        ret.Add("data", "machine server is offline");
        client->response(msg, ret);
        return;
    }

    msg.Add("status", "ready");

    CRCJson * pmsg = new CRCJson(msg);

    std::unique_lock<std::mutex> lock(_task_queue_mtx);
    _task_queue.push(pmsg);
}

bool 
MachineServer::ParseCmd(const CRCJson & json, std::string & cmd) const
{
    std::string&& command = json("cmd");

    if (command.compare(CMD_MAILBOX) == 0)
    {
        std::string && cmdsub = json("cmdsub");
        if (cmdsub.compare("out")==0){
            cmd = "MLOU,0x6001,\n";
            return true;
        }
        if (cmdsub.compare("in")==0){
            cmd = "MLIN,0x6001,\n";
            return true;
        }
    }

    if (command.compare(CMD_CDROM))
    {

    }

    if (command.compare(CMD_PRINTER))
    {
        
    }

    if (command.compare(CMD_QUERY))
    {
        
    }

    return false;

}

bool 
MachineServer::OnProcess4Equipment(CRCJson * pMsg, std::string& str4Eqpt)
{
    CRCJson ret;
    ret.Add("data", str4Eqpt);
    _csCtrl.response(*pMsg, ret);    
}