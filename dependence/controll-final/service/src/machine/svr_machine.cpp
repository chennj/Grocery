#include "svr_machine.h"
#include "svr_utils.h"

MachineServer::~MachineServer()
{
    while(!_task_queue.empty()){
        delete _task_queue.front();
        _task_queue.pop();
    }

    while(!_local_task_queue.empty()){
        delete _local_task_queue.front();
        _local_task_queue.pop();
    }
}

void 
MachineServer::Init()
{
    //第一次启动，设为盘点状态
    m_current_state = INIT;
    
    //启动时获取设备信息的任务加入本地队列，确保是第一个执行的任务
    AddLocalTask(UPDATEMIDASBOX);

    //连接总控,注册服务
    _csCtrl.set_groupid("0001");

    _csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    _csCtrl.reg_msg_call("onopen", std::bind(&MachineServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    _csCtrl.reg_msg_call(CMD_MAILBOX, std::bind(&MachineServer::cs_machine_mailbox, this, std::placeholders::_1, std::placeholders::_2));

    _csCtrl.reg_msg_call(CMD_QUERY, std::bind(&MachineServer::cs_machine_query, this, std::placeholders::_1, std::placeholders::_2));

    //设置处理来自设备的信息的回调处理函数
    _csMachine.onmessage = [this](CRCClientCTxt* pTxtClient){

        std::string& ss = pTxtClient->getContent();
        
        //先行判断是否是at91推送过来的消息
        //如果有那坑定是发生了异常
        bool isException = false;
        if (!isException && strstr(ss.c_str(), DOOROPENREQUEST)){
            CRCLog_Warring("DOOROPENREQUEST......");
            isException = true;
        }

        if (!isException && strstr(ss.c_str(), MAGPLUGOUT)){
            CRCLog_Warring("MAGPLUGOUT......");
            isException = true;
        }

        if (!isException && strstr(ss.c_str(), DOOROPEN)){
            CRCLog_Warring("DOOROPEN......");
            isException = true;
        }

        if (!isException && strstr(ss.c_str(), MAGPLUGIN)){
            CRCLog_Warring("MAGPLUGIN......");
            isException = true;
        }

        if (!isException && strstr(ss.c_str(), DOORCLOSE)){
            CRCLog_Warring("DOORCLOSE......");
            AddLocalTask(UPDATEMIDASBOX);
            m_current_state = INVENTORY;
        }

        if (isException){
            m_current_state = EXCEPTION;
            return;
        }

        OnProcess4Equipment(ss, pTxtClient);
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
    int r_size = CRCConfig::Instance().getInt("nRecvBuffSize", 65536);

    CRCLog_Info("RECVBUFF Size: %d",r_size);

    _csMachine.send_buff_size(s_size);
    _csMachine.recv_buff_size(r_size);        

    while(pThread->isRun())
    {
        //连接有效
        if (_csMachine.isRun())
        {
            //完成一次收发
            _csMachine.OnRun();

            //检查当前状态
            //-----------------------------------------------
            if (m_current_state == EXCEPTION){          //异常状态，全部暂停
                CRCThread::Sleep(1);
                continue;
            }

            if (m_current_state == INIT){               //初始化状态，等待认证结束
               if (_csMachine.isAuth()){
                    m_current_state = RUN;
               }
               continue;
            }
            
            if (m_current_state == INVENTORY){          //盘点状态，返回所有总控任务，只执行本地盘点任务
                //返回总控所有的任务
                while(!_task_queue.empty())
                {
                    CRCJson * pjson = nullptr;
                    {
                        std::lock_guard<std::mutex> lock(_task_queue_mtx);
                        pjson = _task_queue.front();
                        _task_queue.pop();
                    }
                    CRCJson ret;
                    ret.Add("data", "INVENTORY");
                    _csCtrl.response(*pjson, ret);    
                    delete pjson;
                }
            }
            //-----------------------------------------------

            //如果有本地任务，优先执行
            //-----------------------------------------------
            //先判断是否有已经发出的总控任务还没有返回
            if (!_task_queue.empty())
            {
                CRCJson* pmsg = _task_queue.front();
                if ((*pmsg)("status").compare("sent") == 0){
                    CRCThread::Sleep(1);
                    continue;
                }
            }
            //发送本地任务
            if (!_local_task_queue.empty())
            {
                CRCJson*    pmsg    = _local_task_queue.front();
                std::string cmd     = (*pmsg)("cmd");

                //任务已发送还未返回
                if ((*pmsg)("status").compare("sent") == 0){
                    CRCThread::Sleep(1);
                    continue;
                }

                if (SOCKET_ERROR == _csMachine.writeText(cmd.c_str(), cmd.length())){
                    CRCLog_Error("MachineLoop::connect local task send failed");
                    CRCThread::Sleep(1000);
                } else {
                    //修改发送任务为已发送
                    pmsg->Replace("status", "sent");
                }

                continue;
            }
            //-----------------------------------------------

            //发送总控发送过来的任务
            //-----------------------------------------------
            if (!_task_queue.empty())
            {
                CRCJson* pmsg = _task_queue.front();
                {
                    //不做写动作，不用加锁
                    //std::lock_guard<std::mutex> lock(_task_queue_mtx);
                    //msg = _task_queue.front();
                }

                //任务已发送还未返回
                if ((*pmsg)("status").compare("sent") == 0){
                    CRCThread::Sleep(1);
                    continue;
                }

                //检查命令是否合法
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
                    CRCLog_Error("MachineLoop COMMAND<%s> SEND FAILED",cmd.c_str());
                    CRCThread::Sleep(1000);
                }
                
            } else {
                //没有任务，睡会儿
                CRCThread::Sleep(5);
            }
            //-----------------------------------------------
            continue;           
        }

        //重连设备服务器
        if (_csMachine.connect(AF_INET,"111.111.1.100", 2020))
        {
            CRCLog_Info("MachineLoop::connect machine success.");
            continue;
        }

        //重连失败，睡会儿
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
        ret.Add("data", "equiptment client is offline");
        client->response(msg, ret);
        return;
    }

    if (!_csMachine.isRun()){
        ret.Add("data", "equiptment server is offline");
        client->response(msg, ret);
        return;
    }

    msg.Add("status", "ready");

    CRCJson * pmsg = new CRCJson(msg);

    std::unique_lock<std::mutex> lock(_task_queue_mtx);
    _task_queue.push(pmsg);
}

void 
MachineServer::cs_machine_query(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("MachineServer::cs_machine_query msg: %s", msg.ToString().c_str());

    CRCJson ret;
    if (!_thread.isRun())
    {
        ret.Add("data", "equiptment client is offline");
        client->response(msg, ret);
        return;
    }

    if (!_csMachine.isRun()){
        ret.Add("data", "equiptment server is offline");
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

void 
MachineServer::AddLocalTask(LocalTaskType localTaskType)
{
    switch(localTaskType)
    {
        case UPDATEMIDASBOX:
        {
            CRCJson * pMsg = new CRCJson();
            pMsg->Add("cmd",GMAP);
            pMsg->Add("status", "ready");
            std::unique_lock<std::mutex> lock(_local_task_queue_mtx);
            _local_task_queue.push(pMsg);
            break;
        }
        default:break;
    }
}

void 
MachineServer::OnProcess4Equipment(std::string& str4Eqpt, CRCClientCTxt* pTxtClient)
{
    CRCJson* pmsg  = nullptr;
    
    //先处理本地任务
    if (!_local_task_queue.empty())
    {
        //判断返回的是否本地发送的任务
        pmsg  = _local_task_queue.front();
        if ((*pmsg)("status").compare("sent") == 0){
            CRCLog_Info("update eqpt info...");
            {
                std::unique_lock<std::mutex> lock(_local_task_queue_mtx);
                _local_task_queue.pop();
            }
            update_storage_info(str4Eqpt, pTxtClient);
            return;
        }
    }

    //处理总控发过来的请求
    if (!_task_queue.empty())
    {
        std::lock_guard<std::mutex> lock(_task_queue_mtx);
        pmsg = _task_queue.front();
        _task_queue.pop();
    }

    CRCLog_Info("_csMachine.onmessage: recv %s; msg %s",str4Eqpt.c_str(), pmsg->ToString().c_str());

    CRCJson ret;
    ret.Add("data", str4Eqpt);
    _csCtrl.response(*pmsg, ret);    
}

void
MachineServer::update_storage_info(std::string& ss, CRCClientCTxt* pTxtClient)
{
    //来之at91的 MidasBox
    //记录着所有设备信息
    MidasBox*   pMidasBox   = nullptr;
    int         datalen     = pTxtClient->getRecvLen();

    //通过长度判断返回的是否 MidasBox
    if (datalen != (sizeof(MidasBox)+7)){
        CRCLog_Error("update_eqpt_info recv\n<%s>\n isn't midasbox",ss.c_str());
        return;
    }
    pMidasBox = (MidasBox*)(ss.c_str()+6);
    if (!pMidasBox){
        CRCLog_Error("update_eqpt_info convert midasbox failed");
        return;
    }

    memcpy(
        m_storage.mag_slotarray,
        pMidasBox->mag_slotarray,
        sizeof(MidasMagSlot)*MAGSLOTMAX
    );
    //更新盘槽状态 ,用于盘槽状态发生变化 需要重新更新状态
    CRCLog_Info("update mag slot ...");
    for (int i = 0; i < MAGSLOTMAX; i++)
    {
        if (m_storage.mag_slotarray[i].mag_plug == 0)
        {
            printf("## %d mag is no plug !\n",i);
            for (int j = 0;j < MAGITEMMAX; j++)
            {
                m_storage.media_attr[i * MAGITEMMAX + j].media_addr = i * MAGITEMMAX + j +1;
                m_storage.media_attr[i * MAGITEMMAX + j].cdexist    = NOPRESENCE;
                m_storage.media_attr[i * MAGITEMMAX + j].trayexist  = NOPRESENCE;
                m_storage.media_attr[i * MAGITEMMAX + j].ischecked  = 0;
                m_storage.media_attr[i * MAGITEMMAX + j].isblank    = 0;
                
                memset(&(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo),0,sizeof(DiskTypeInfo));
                /* clean mirror */
                char cmd_buff[256] = {0};
                sprintf(cmd_buff,"mv %s/%04d* %s 2>/dev/null",SVRUTILS::MIRRORPATH,i*MAGITEMMAX+j +1,SVRUTILS::TRASHPATH);
                SVRUTILS::SystemExec(cmd_buff);
                
                /* clean info & grep 离线后文件改名为1-50相对地址  */
                if (strlen(m_storage.mag_slotarray[i].magazine.serial) > 0)
                {
                    snprintf(
                        cmd_buff,sizeof(cmd_buff),
                        "mv -f %s/%04d-%s.grep  %s/%04d-%s.offline",
                        SVRUTILS::DINFOPATH, 
                        i * MAGITEMMAX + j + 1,
                        m_storage.mag_slotarray[i].magazine.serial,
                        SVRUTILS::DINFOPATH,
                        j + 1,
                        m_storage.mag_slotarray[i].magazine.serial);
                }else{
                    snprintf(cmd_buff,sizeof(cmd_buff),"rm -Rf %s/%04d*",SVRUTILS::DINFOPATH,i * MAGITEMMAX + j + 1);
                }
                SVRUTILS::SystemExec(cmd_buff);

            }//!for (int j = 0;j < MAGITEMMAX; j++)

        } else {

            printf("## %d mag is  plug !,seral:%s\n",i,m_storage.mag_slotarray[i].magazine.serial);
            for (int j = 0; j < MAGITEMMAX; j++)
            {
                m_storage.media_attr[i*MAGITEMMAX+j].media_addr = i*MAGITEMMAX+j +1;
                m_storage.media_attr[i*MAGITEMMAX+j].cdexist    = m_storage.mag_slotarray[i].magazine.mag_item[j].blue_cd.cd_presence_flag;
                m_storage.media_attr[i*MAGITEMMAX+j].trayexist  = m_storage.mag_slotarray[i].magazine.mag_item[j].mag_tray.tray_presence_flag;
                //设置所有插槽的空闲状态
                m_storage.media_attr[i*MAGITEMMAX+j].slot_status = 'N';  

                CRCLog_Info("debug:addr:%d,cdexist:%d,trayexist:%d,ischecked:%d,isblank:%d,isappen:%d,freesize:%012llu,mediatype %s",
                    m_storage.media_attr[i*MAGITEMMAX+j].media_addr,
                    m_storage.media_attr[i*MAGITEMMAX+j].cdexist,
                    m_storage.media_attr[i*MAGITEMMAX+j].trayexist,
                    m_storage.media_attr[i*MAGITEMMAX+j].ischecked,
                    m_storage.media_attr[i*MAGITEMMAX+j].isblank,
                    m_storage.media_attr[i*MAGITEMMAX+j].isappendable,
                    m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_track_freesize,
                    m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str
                );
            }//!for (int j = 0;j < MAGITEMMAX; j++)

        }//!(pMidasBox->mag_slotarray[1].mag_plug == 0)
    }

    //更新光驱属性
    CRCLog_Info("update recorder ...");
    for (int i =0; i< pMidasBox->midas_attr.rec_max;i++)
    {
        if (pMidasBox->recorde_slotArray[i].recorder_plug)
        {
            m_storage.record_attr[i].is_pluging     = 1;
            m_storage.record_attr[i].record_address = 0x5001+i;
            m_storage.record_attr[i].is_have_media  = pMidasBox->recorde_slotArray[i].recorder.blue_cd.cd_presence_flag;
            m_storage.record_attr[i].media_address  = pMidasBox->recorde_slotArray[i].recorder.cd_src_address;
            m_storage.record_attr[i].is_busy        = pMidasBox->recorde_slotArray[i].recorder_busy;
            //记录更新时间
            time(&(m_storage.record_attr[i].times)); 
        }else{
            m_storage.record_attr[i].is_pluging     = 0;
            m_storage.record_attr[i].record_address = 0x5001+i;
        }
        
    }

}