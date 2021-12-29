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
    _csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    _csCtrl.reg_msg_call("onopen", std::bind(&MachineServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    _csCtrl.reg_msg_call("0001:cs_machine_mailbox", std::bind(&MachineServer::cs_machine_mailbox, this, std::placeholders::_1, std::placeholders::_2));

    _csMachine.onmessage = [this](CRCClientCTxt* pTxtClient){

        std::string& ss = pTxtClient->getContent();
        
        CRCJson* pmsg = nullptr;
        {
            std::lock_guard<std::mutex> lock(_task_queue_mtx);
            pmsg = _task_queue.front();
            _task_queue.pop();
        }

        CRCLog_Info("_csMachine.onmessage: recv %s; msg %s",ss.c_str(), pmsg->ToString().c_str());

        CRCJson ret;
        ret.Add("data", ss);
        _csCtrl.response(*pmsg, ret);
        delete pmsg;
    };

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

    while(pThread->isRun()){

        if (_csMachine.isRun())
        {
            _csMachine.OnRun();
            //如果有任务
            if (!_task_queue.empty())
            {
                CRCJson& msg = *_task_queue.front();
                {
                    //不做写动作，不用加锁
                    //std::lock_guard<std::mutex> lock(_task_queue_mtx);
                    //msg = _task_queue.front();
                }

                if (msg("status").compare("ready") != 0){
                    continue;
                }

                std::string cmdsub = msg("cmdsub");
                if (cmdsub.compare("out")==0){
                    cmdsub = "MLOU,0x6001,\n";
                }else {
                    cmdsub = "MLIN,0x6001,\n";
                }

                if (SOCKET_ERROR != _csMachine.writeText(cmdsub.c_str(), cmdsub.length())){
                    msg.Replace("status", "sent");
                } else {       
                    CRCThread::Sleep(1000);
                }
                
                continue;
            }
            //如果没有任务
            if (_task_queue.empty())
            {
                CRCThread::Sleep(1);
                continue;
            }
            
        }

        if (_csMachine.connect(AF_INET,"111.111.1.100", 2020))
        {
            CRCLog_Warring("Machine::connect() success.");
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
    json.Add("groudid", "0001");
    json.AddEmptySubArray("apis");
    json["apis"].Add("cs_machine_cdrom");
    json["apis"].Add("cs_machine_mailbox");
    json["apis"].Add("cs_machine_printer");

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