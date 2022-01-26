#include "svr_machine.h"
#include "svr_utils.h"

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
    //第一次启动，设为盘点状态
    m_current_state = INIT;
    
    //初始化线程池并启动
    m_thread_pool.init(4);
    m_thread_pool.start();

    //连接总控,注册服务
    _csCtrl.set_groupid("0001");

    _csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    _csCtrl.reg_msg_call("onopen", std::bind(&MachineServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    _csCtrl.reg_msg_call(CMD_ACTION, std::bind(&MachineServer::cs_machine_action, this, std::placeholders::_1, std::placeholders::_2));

    //启动时获取设备信息的任务加入本地队列，确保是第一个执行的任务
    do_action(UPDATEMIDASBOX);

    //设置处理来自设备的信息的回调处理函数
    _csMachine.onmessage = [this](CRCClientCTxt* pTxtClient){

        char* ss = pTxtClient->getRecvData();
        
        //先行判断是否是at91推送过来的消息
        //如果有那坑定是发生了异常
        bool isException = false;
        if (!isException && strstr(ss, DOOROPENREQUEST)){
            CRCLog_Warring("DOOROPENREQUEST......");
            isException = true;
        }

        if (!isException && strstr(ss, MAGPLUGOUT)){
            CRCLog_Warring("MAGPLUGOUT......");
            isException = true;
        }

        if (!isException && strstr(ss, DOOROPEN)){
            CRCLog_Warring("DOOROPEN......");
            isException = true;
        }

        if (!isException && strstr(ss, MAGPLUGIN)){
            CRCLog_Warring("MAGPLUGIN......");
            isException = true;
        }

        if (!isException && strstr(ss, DOORCLOSE)){
            CRCLog_Warring("DOORCLOSE......");
            m_current_state = INVENTORY;
            do_action(UPDATEMIDASBOX);
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
            //-----------------------------------------------
            _csMachine.OnRun();

            if (!_csMachine.isAuth()){
                continue;
            }

            //检查当前状态
            //-----------------------------------------------
            if (m_current_state == EXCEPTION){          //异常状态，全部暂停
                CRCThread::Sleep(1);
                continue;
            }            

            //发送的任务
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
                std::string&& cmd = (*pmsg)("cmdTransfer");
                
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
        //-----------------------------------------------
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
    json["apis"].Add("cs_machine_action");

    client->request("ss_reg_api", json, [](CRCNetClientC* client, CRCJson& msg) {
        CRCLog_Info("MachineServer::ss_reg_api return: %s", msg("data").c_str());
    });
}

void 
MachineServer::cs_machine_action(CRCNetClientC* client, CRCJson& msg)
{
    CRCLog_Info("MachineServer::cs_machine_action msg: %s", msg.ToString().c_str());

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

    if (m_current_state == INIT){               //初始化状态，等待认证结束，并进入初始化流程
        ret.Add("data", "INIT");
        client->response(msg, ret);
        return;
    }

    if (m_current_state == INVENTORY){          //盘点状态，返回所有总控任务，进入盘点流程
        ret.Add("data", "INVENTORY");
        client->response(msg, ret);
        return;
    }

    msg.Add("status", "ready");

    do_action(NORMAL, &msg);
}

void 
MachineServer::do_action(MachineTaskType taskType, CRCJson * pJson)
{
    switch(taskType)
    {
        case UPDATEMIDASBOX:
        {
            m_thread_pool.exec(std::bind(&MachineServer::update_storage_info, this));
            break;
        }
        case NORMAL:
        {
            std::string&& command   = (*pJson)("cmdsub");
            if (command.empty()){
                CRCJson ret;
                ret.Add("data", "cmd lost");
                _csCtrl.response(*pJson, ret);                
            }

            CRCJson * pMsg          = new CRCJson(*pJson);
            if (command.compare(MAILBOXIN) == 0){
                m_thread_pool.exec(std::bind(&MachineServer::mailbox_in, this, std::placeholders::_1), pMsg);
            }
            if (command.compare(MAILBOXOUT) == 0){
                m_thread_pool.exec(std::bind(&MachineServer::mailbox_out, this, std::placeholders::_1), pMsg);
            }
            break;
        }
        default:break;
    }
}

void 
MachineServer::OnProcess4Equipment(const char* pData, CRCClientCTxt* pTxtClient)
{
    CRCJson* pMsg  = nullptr;

    //处理设备发过来的请求
    if (!_task_queue.empty()){

        {
            std::lock_guard<std::mutex> lock(_task_queue_mtx);
            pMsg = _task_queue.front();
            _task_queue.pop();
        }

        RetMessage* rm = new RetMessage(*pMsg, pData, pTxtClient->getRecvLen());

        std::string&& key = gen_key(*pMsg);

        CRCLog_Info ("OnProcess4Equipment: recv %s; recvlen %d; key %s",pData, strlen(pData), key.c_str());

        {
            std::unique_lock<std::mutex> lock(m_result_map_mtx);
            m_result_map.insert(std::map<std::string, RetMessage*>::value_type(key, rm)); 
            m_condition.notify_all();
        }

        delete pMsg;

    } else {

        CRCLog_Error("OnProcess4Equipment: recv %s; \nEXCEPTION: %s",pData, "No corresponding request found");
        //CRCJson ret;
        //ret.Add("data", str4Eqpt);
        //_csCtrl.response(*pmsg, ret);    
    }
}


void
MachineServer::update_storage_info()
{
    //来之at91的 MidasBox
    //记录着所有设备信息
    MidasBox*   pMidasBox   = nullptr;
    int         datalen     = 0;
    char        cmdBuf[64];
    FILE*       file        = NULL;
    std::string from        = "local_update_storage_info";
    RetMessage* rm          = nullptr;

    CRCJson *   pMsg        = new CRCJson();
    uint32_t    fromId      = m_atomic++;

    //生成任务对象
    pMsg->Add("cmdTransfer",    GMAP);
    pMsg->Add("from",           from);
    pMsg->Add("fromId",         fromId);
    pMsg->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pMsg);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(_task_queue_mtx);
        _task_queue.push(pMsg);
    }

    //等待结果返回
    CRCLog_Info("update_storeage_info WAITTING msg key<%s> return ...", key.c_str());

    //不使用轮询，使用用 condition
    /*
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    while(true)
    {
        std::map<std::string, RetMessage*>::iterator iter;
        iter = m_result_map.find(key);
        if (iter != m_result_map.end()){
            {
                std::unique_lock<std::mutex> lock(m_result_map_mtx);
                rm = iter->second;
                m_result_map.erase(iter);
            }
            break;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            CRCLog_Info("update_storeage_info find key %s", key.c_str());
            continue;
        }
    }
    */
    /* 放到公共函数里
    {
        std::unique_lock<std::mutex> lock(m_result_map_mtx);
        std::map<std::string, RetMessage*>::iterator iter;
        bool no_timeout = m_condition.wait_for(lock, std::chrono::milliseconds(1000*10), [&,this]{

            if (!this->m_result_map.empty()){
                iter = this->m_result_map.find(key);
                return iter != this->m_result_map.end();
            } else {
                return false;
            }

        });
        if (!no_timeout){
            CRCLog_Error("update_eqpt_info recv timeout");
            return;
        }
        rm = iter->second;
        m_result_map.erase(iter);
    }
    */
    if (!wait_for(key, "update_eqpt_info recv timeout", rm))
    {
        return;
    }

    //通过长度判断返回的是否 MidasBox
    if (rm->datalen != (sizeof(MidasBox)+7)){
        CRCLog_Error("update_eqpt_info recv\n<%s>\n isn't midasbox",rm->data);
        return;
    }

    pMidasBox = (MidasBox*)( rm->data+6);
    if (!pMidasBox){
        CRCLog_Error("update_eqpt_info convert midasbox failed");
        return;
    }

    memcpy(
        m_storage.mag_slotarray,
        pMidasBox->mag_slotarray,
        sizeof(MidasMagSlot)*MAGSLOTMAX
    );

    delete rm;

    //更新盘槽状态 ,用于盘槽状态发生变化 需要重新更新状态
    CRCLog_Info("update mag slot ...");
    for (int i = 0; i < 12/*MAGSLOTMAX*/; i++)      //MAGSLOTMAX太大了，现实没有这么多的盘仓，12就足够了
    {
        if (m_storage.mag_slotarray[i].mag_plug == 0)
        {
            CRCLog_Info("## %d mag is no plug !\n",i);
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

            CRCLog_Info("## %d mag is  plug !,seral:%s\n",i,m_storage.mag_slotarray[i].magazine.serial);
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

    m_current_state = RUN;

    return;

	//查找光驱对应的设备名 host为空表示是SATA接口 否则HOST表示PCI 的HBA卡位置
	if (strlen(m_storage.scsi_host) > 0)
	{
		sprintf(cmdBuf,"find /sys/devices/ -name \"sr[0-9]*\" |grep %s >/tmp/record.txt",m_storage.scsi_host);
	}else{
		sprintf(cmdBuf,"find /sys/devices/ -name \"sr[0-9]*\"  >/tmp/record.txt");
	}
	SVRUTILS::SystemExec(cmdBuf);
	file = fopen("/tmp/record.txt","r");
	if (file !=NULL)
	{
		char read_buff[256];
		while(!feof(file))
        {
			if (fgets(read_buff,sizeof(read_buff),file)){
				char *tmp   = NULL;
				char *tmp2  = NULL;
				CRCLog_Info("####read_buf :%s\n",read_buff);
				tmp =  strstr(read_buff,"sr");
				if (tmp){
					tmp2 = strchr(tmp,'\n');
					if (tmp2)
					{
						*tmp2 = 0;
					}
					//顺序查找光驱
					for (int i = 0;i<RECORDERMAX;i++)
					{
                        if (strlen(m_storage.record_attr[i].dev_name)==0)
                        {
                            sprintf(m_storage.record_attr[i].dev_name,"/dev/%s",tmp);
                            break;
                        }
					}
					
				}				

			}
		}//!while(!feof(file))
		pclose(file);
	}else{
		CRCLog_Warring("Cann't  execute popen cmd!\n");
	}//!if (file !=NULL)

    //检查光驱
	for (int i =0; i<RECORDERMAX; i++)
	{
		if (m_storage.record_attr[i].is_pluging)
		{
			CRCLog_Info("%d record,address 0x%04x,devname:%s\n",i,m_storage.record_attr[i].record_address,m_storage.record_attr[i].dev_name);
			if (strlen(m_storage.record_attr[i].dev_name) <= 0)
			{
				CRCLog_Error("FOUND CRITIC ERROR!,can't find any cdrom devname,check config.xml sas hba host is correct! \n");
                CRCLog_Error("Machine Server Stop...\n\n");	
				exit(-1);
			}
		}
		
	}
}

void
MachineServer::inventory_all()
{
    if (!_isAutoInventory){
        CRCLog_Info("inventory return, becase auto inventory is false");
        return;
    }
	for (int i=0; i<MAGSLOTMAX; i++)
	{
		if (m_storage.mag_slotarray[i].mag_plug == 1)
		{
			for (int j=1;j<=MAGITEMMAX;j++){
				if ((m_storage.media_attr[i*MAGITEMMAX+j].cdexist == PRESENCE) &&
					(m_storage.media_attr[i*MAGITEMMAX+j].ischecked !=1) &&
					(m_storage.media_attr[i*MAGITEMMAX+j].slot_status == 'N'))
				{
                    /*
					EsPollEvent eve;
					eve.EsType = GETDISKINFO;
					eve.parm.get_disk_info.cdaddr = i*50+j+1; 	//光盘地址
					eve.callback = GetDiskInfo;					//回调函数
					EpollPostEvent(&eve);	//送入事件队列
                    */
				}
				
			}
		}			
	}

}

void
MachineServer::inventory_one(uint32_t cd_addr)
{

}

void 
MachineServer::mailbox_in(CRCJson * pJson)
{
    std::string from        = "remote_mailbox_in";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;

    //生成任务对象
    pJson->Add("cmdTransfer",    MAILBOXIN_TRANSFER);
    pJson->Add("from",           from);
    pJson->Add("fromId",         fromId);
    pJson->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pJson);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(_task_queue_mtx);
        _task_queue.push(pJson);
    }

    CRCLog_Info("mailbox_in WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "mailbox_in recv timeout", rm))
    {
        return;
    }

    CRCJson ret;
    ret.Add("data", rm->data);
    _csCtrl.response(rm->json, ret);    
}

void 
MachineServer::mailbox_out(CRCJson * pJson)
{
    std::string from        = "remote_mailbox_out";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;

    //生成任务对象
    pJson->Add("cmdTransfer",    MAILBOXOUT_TRANSFER);
    pJson->Add("from",           from);
    pJson->Add("fromId",         fromId);
    pJson->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pJson);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(_task_queue_mtx);
        _task_queue.push(pJson);
    }

    CRCLog_Info("mailbox_in WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "mailbox_out recv timeout", rm))
    {
        return;
    }

    CRCJson ret;
    ret.Add("data", rm->data);
    _csCtrl.response(rm->json, ret);   
}

std::string 
MachineServer::gen_key(const CRCJson & json)
{
    std::string key;
    key.append(json("from")).append(":").append(json("fromId"));
    return key;
}

bool 
MachineServer::wait_for(const std::string& key, const std::string& msg, RetMessage*& pRmOut, uint32_t timeout)
{
    std::unique_lock<std::mutex> lock(m_result_map_mtx);
    std::map<std::string, RetMessage*>::iterator iter;
    bool no_timeout = m_condition.wait_for(lock, std::chrono::milliseconds(timeout), [&,this]{

        if (!this->m_result_map.empty()){
            iter = this->m_result_map.find(key);
            return iter != this->m_result_map.end();
        } else {
            return false;
        }

    });
    if (!no_timeout){
        CRCLog_Error(msg.c_str());
        return false;
    }
    pRmOut = iter->second;
    m_result_map.erase(iter);
    return true;
}