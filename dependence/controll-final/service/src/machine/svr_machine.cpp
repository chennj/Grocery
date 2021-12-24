#include "svr_machine.h"

void 
MachineServer::Init()
{
    _csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    _csCtrl.reg_msg_call("onopen", std::bind(&MachineServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    _csCtrl.reg_msg_call("0001:cs_machine_mailbox", std::bind(&MachineServer::cs_machine_mailbox, this, std::placeholders::_1, std::placeholders::_2));

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
    
    _csMachine.onmessage = [this](CRCClientCTxt* pTxtClient){
        std::string ss = pTxtClient->getContent();
        CRCLog_Info("MachineLoop info: %s",ss.c_str());
    };

    while(pThread->isRun()){

        if (_csMachine.isRun())
        {
            _csMachine.OnRun();
            continue;
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

    std::string cmdsub = msg["cmdsub"]("cmdsub");
    std::string ss;
    CRCLog_Info("MachineServer::cs_machine_mailbox cmdsub %s", cmdsub.c_str());
    if (cmdsub.compare("out")==0){
        ss = "MLOU,0x6001,\n";
    }else {
        ss = "MLIN,0x6001,\n";
    }

    CRCLog_Info("MachineServer::cs_machine_mailbox ss %s", ss.c_str());

    _csMachine.writeText(ss.c_str(), ss.length());
    
    CRCJson ret;
    ret.Add("data", "login successs.");
    client->response(msg, ret);
}