#include "svr_machine.h"
#include "svr_utils.h"
#include "svr_comm_dir.h"

#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <mntent.h>

MachineServer::~MachineServer()
{
    while(!m_task_queue.empty()){
        delete m_task_queue.front();
        m_task_queue.pop();
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
    m_csCtrl.set_groupId("0001");

    m_csCtrl.connect("csCtrl","ws://192.168.137.129:4567");

    m_csCtrl.reg_msg_call("onopen", std::bind(&MachineServer::onopen_csCtrl, this, std::placeholders::_1, std::placeholders::_2));

    m_csCtrl.reg_msg_call(CMD_ACTION, std::bind(&MachineServer::cs_machine_action, this, std::placeholders::_1, std::placeholders::_2));

    //启动时获取设备信息的任务加入本地队列，确保是第一个执行的任务
    do_action(UPDATEMIDASBOX);

    //设置处理来自设备的信息的回调处理函数
    m_csMachine.onmessage = [this](CRCClientCTxt* pTxtClient){

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
    //2. 轮询任务列表，发送状态为‘ready’的任务，并将其标记为‘sent’
    //3. 检查当前运行状态
    m_thread.Start(
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
     m_csCtrl.run(1);
}

void 
MachineServer::Close()
{
    m_csCtrl.close();
}

void 
MachineServer::MachineLoop(CRCThread* pThread)
{
    int s_size = CRCConfig::Instance().getInt("nSendBuffSize", 65536*2);
    int r_size = CRCConfig::Instance().getInt("nRecvBuffSize", 65536*2);

    CRCLog_Info("RECVBUFF Size: %d",r_size);

    m_csMachine.send_buff_size(s_size);
    m_csMachine.recv_buff_size(r_size);        

    while(pThread->isRun())
    {
        //连接有效
        if (m_csMachine.isRun())
        {
            //完成一次收发
            //-----------------------------------------------
            m_csMachine.OnRun();

            if (!m_csMachine.isAuth()){
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
            if (!m_task_queue.empty())
            {
                CRCJson* pmsg = m_task_queue.front();
                {
                    //不用加锁
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
                
                if (SOCKET_ERROR != m_csMachine.writeText(cmd.c_str(), cmd.length())){
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
        if (m_csMachine.connect(AF_INET,"111.111.1.100", 2020))
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
    if (!m_thread.isRun())
    {
        ret.Add("data", "equiptment client is offline");
        client->response(msg, ret);
        return;
    }

    if (!m_csMachine.isRun()){
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

    if (m_current_state == RETURNDISC){         //回盘状态，返回所有总控任务，进入回盘流程
        ret.Add("data", "RETURNDISC");
        client->response(msg, ret);
        return;        
    }

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
                m_csCtrl.response(*pJson, ret);                
            }

            //复制pJson
            //因为传递给线程后pJson指向的数据会被释放,所以需要复制一份
            CRCJson * pMsg  = new CRCJson(*pJson);

            if (command.compare(MAILBOXIN) == 0){
                m_thread_pool.exec(std::bind(&MachineServer::mailbox_in, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(MAILBOXOUT) == 0){
                m_thread_pool.exec(std::bind(&MachineServer::mailbox_out, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(INVENTORYTEST) == 0){
                //改变状态：盘点
                m_current_state = INVENTORY;
                //盘点入池
                m_thread_pool.exec(std::bind(&MachineServer::inventory, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(QUERY_STATION) == 0){
                //获取站点信息
                m_thread_pool.exec(std::bind(&MachineServer::query_station, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(RETURN_DISC) == 0){
                //改变状态：回盘
                m_current_state = RETURNDISC;
                //获取站点信息
                m_thread_pool.exec(std::bind(&MachineServer::cdrom_return_disc_manual, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(MAILBOX_EXPORT_DISC) == 0){
                //邮箱出盘
                m_thread_pool.exec(std::bind(&MachineServer::mailbox_export_disc_manual, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(MAILBOX_IMPORT_DISC) == 0){
                //邮箱入盘
                m_thread_pool.exec(std::bind(&MachineServer::mailbox_import_disc_manual, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(PRINTER_EXPORT_DISC) == 0){
                //打印机回盘
                m_thread_pool.exec(std::bind(&MachineServer::printer_export_disc_manual, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(PRINTER_IMPORT_DISC) == 0){
                //光盘送打印机
                m_thread_pool.exec(std::bind(&MachineServer::printer_import_disc_manual, this, std::placeholders::_1), pMsg);
            }

            if (command.compare(MOVE_DISC_TO_CDROM) == 0){
                //送光盘至空闲光驱
                m_thread_pool.exec(std::bind(&MachineServer::move_auto_disc2cdrom, this, std::placeholders::_1), pMsg);
            }
            break;
        }
        case AUTOINVENTORY:
        {
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
    if (!m_task_queue.empty()){

        {
            std::lock_guard<std::mutex> lock(m_task_queue_mtx);
            pMsg = m_task_queue.front();
            m_task_queue.pop();
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
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pMsg);
    }

    //等待结果返回
    CRCLog_Info("update_storeage_info WAITTING msg key<%s> return ...", key.c_str());

    /* 不使用轮询，使用 condition，逼格高
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
    }*/

    /* 放到函数里，共用
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
    }*/

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
        (void*)m_storage.mag_slotarray,
        pMidasBox->mag_slotarray,
        sizeof(MidasMagSlot)*MAGSLOTMAX
    );

    memcpy(
        (void*)&m_storage.midas_box,
        pMidasBox,
        sizeof(MidasBox)
    );

    delete rm;

    //更新盘槽状态 ,用于盘槽状态发生变化 需要重新更新状态
    CRCLog_Info("update mag slot ...");
    CRCLog_Info("-----------------------------------------------------------------------------------");
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
                
                memset((void*)&(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo),0,sizeof(SDiskTypeInfo));
                /* clean mirror */
                char cmd_buff[256] = {0};
                sprintf(cmd_buff,"mv %s/%04d* %s 2>/dev/null",SVRUTILS::MIRRORPATH,i*MAGITEMMAX+j +1,SVRUTILS::TRASHPATH);
                SVRUTILS::SystemExec(cmd_buff);
                
                /* clean info & grep 离线后文件改名为1-50相对地址  */
                if (strlen(const_cast<char*>(m_storage.mag_slotarray[i].magazine.serial)) > 0)
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
    CRCLog_Info("-----------------------------------------------------------------------------------");
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
            time((time_t*)&(m_storage.record_attr[i].times)); 
        }else{
            m_storage.record_attr[i].is_pluging     = 0;
            m_storage.record_attr[i].record_address = 0x5001+i;
        }

        CRCLog_Info("debug:isplug:%d,drvaddr:0x%04x,ishavecd:%d,cdaddr:0x%04x,isbusy:%d",
            m_storage.record_attr[i].is_pluging,
            m_storage.record_attr[i].record_address,
            m_storage.record_attr[i].is_have_media,
            m_storage.record_attr[i].media_address,
            m_storage.record_attr[i].is_busy
        );
        
    }

    m_current_state = RUN;

    cdrom_return_disc();
    return;

	//查找光驱对应的设备名 host为空表示是SATA接口 否则HOST表示PCI 的HBA卡位置
	if (strlen(const_cast<char*>(m_storage.scsi_host)) > 0)
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
                        if (strlen(const_cast<char*>(m_storage.record_attr[i].dev_name))==0)
                        {
                            sprintf(const_cast<char*>(m_storage.record_attr[i].dev_name),"/dev/%s",tmp);
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
			if (strlen(const_cast<char*>(m_storage.record_attr[i].dev_name)) <= 0)
			{
				CRCLog_Error("FOUND CRITIC ERROR!,can't find any cdrom devname,check config.xml sas hba host is correct! \n");
                CRCLog_Error("Machine Server Stop...\n\n");	
				exit(-1);
			}
		}
		
	}

    //将光驱的盘放回盘仓
    cdrom_return_disc();
}

void
MachineServer::inventory(CRCJson * pJson)
{
    CRCThreadPool pool;
    pool.init(3);
    pool.start();

	for (int i = 0; i < MAGSLOTMAX; i++)
	{
		if (m_storage.mag_slotarray[i].mag_plug == 1)
		{
			for (int j=1; j<= MAGITEMMAX; j++){
				if ((m_storage.media_attr[i*MAGITEMMAX+j].cdexist == PRESENCE) &&
					(m_storage.media_attr[i*MAGITEMMAX+j].ischecked !=1) &&
					(m_storage.media_attr[i*MAGITEMMAX+j].slot_status == 'N'))
				{   
                    pool.exec(std::bind(&MachineServer::get_discinfo, this, std::placeholders::_1,std::placeholders::_2), i*MAGITEMMAX+j+1, true );
                    //先测试盘点一张盘
                    //break;
				}
				
			}
		}			
	}
    
    pool.waitForAllDone();
    pool.stop();

    CRCLog_Info("盘点结束");
    
    CRCJson ret;
    ret.Add("data", "盘点结束");
    m_csCtrl.response(*pJson, ret);    

    //释放
    delete pJson;

    m_current_state = RUN;

}

void
MachineServer::get_discinfo(uint32_t cd_addr, bool is_returndisc)
{
    int  try_num = 1;
    int  ret = -1;
    char cmd_buf[256];

    CRCLog_Info("START process GetDiskInfo = %d",cd_addr);

    m_storage.media_attr[cd_addr-1].slot_status = 'P';

    //如果盘没有读到 重新盘点一次
    for (int i = 0; i < try_num; i++)
    {
        int cdrom_addr = 0;
        MOVEDISCTOCDROM:
        {
            //为整个动作枷锁
            std::unique_lock<std::mutex> lock(m_action_mtx);            
            cdrom_addr = move_disc2cdrom(cd_addr);
        }
        if (cdrom_addr == 0){
            //如果没有找到可用的光驱，睡一会儿再试；
            CRCLog_Info("----WAITTING FOR AVAILABLE FREE CDROM FOR %d",cd_addr);
            CRCThread::Sleep(1000*2);
            goto MOVEDISCTOCDROM;
        }

        if (cdrom_addr == NODISC){
            //delete mirror fs
            sprintf(cmd_buf,"rm -Rf %s/%04d_*",SVRUTILS::LOGPATH,cd_addr);
            SVRUTILS::SystemExec(cmd_buf);
            break;
        }
        if (cdrom_addr > 0){
            /*测试，不检查光盘信息
            CRCLog_Debug("get_discinfo: %d before check_cd_cdrom(cd_addr, record_addr, 0)\n",__LINE__);
            ret = check_cd_cdrom(cd_addr, cdrom_addr, 0);
            CRCLog_Debug("get_discinfo: %d after  check_cd_cdrom(cd_addr, record_addr, 0)\n",__LINE__);

            if (ret == NOINFO)
            {
                printf("get_discinfo: CANN'T Get %d disc info,retry %d!\n",cd_addr,i);
            }

            if (ret == -1){
				CRCLog_Error("get_discinfo: analysis disc <%d> info exception", cd_addr);
			}            
            */
            std::this_thread::sleep_for(std::chrono::milliseconds((15 + rand() % 30)*1000));

            if (is_returndisc)
            {
                CRCLog_Debug("%d before cdrom_return_disc(cdrom_addr)\n",__LINE__);
                ret = cdrom_return_disc(cdrom_addr);
                CRCLog_Debug("%d after  cdrom_return_disc(cdrom_addr)\n",__LINE__);
            }

        }
    }

    CRCLog_Info("END process GetDiskInfo = %d",cd_addr);
}

void 
MachineServer::mailbox_in(CRCJson * pJson)
{
    std::string from        = "remote_mailbox_in";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;
    CRCJson     ret;

    //生成任务对象
    pJson->Add("cmdTransfer",    MAILBOXIN_TRANSFER);
    pJson->Add("from",           from);
    pJson->Add("fromId",         fromId);
    pJson->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pJson);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pJson);
    }

    CRCLog_Info("mailbox_in WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "mailbox_in recv timeout", rm))
    {
        ret.Add("data", "mailbox_in recv timeout");
        m_csCtrl.response(rm->json, ret);    
        delete pJson;
        return;
    }

    ret.Add("data", rm->data);
    m_csCtrl.response(rm->json, ret);    
    delete rm;
}

void 
MachineServer::mailbox_out(CRCJson * pJson)
{
    std::string from        = "remote_mailbox_out";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;
    CRCJson     ret;    

    //生成任务对象
    pJson->Add("cmdTransfer",    MAILBOXOUT_TRANSFER);
    pJson->Add("from",           from);
    pJson->Add("fromId",         fromId);
    pJson->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pJson);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pJson);
    }

    CRCLog_Info("mailbox_in WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "mailbox_out recv timeout", rm))
    {
        ret.Add("data", "mailbox_out recv timeout");
        m_csCtrl.response(rm->json, ret);    
        delete pJson;
        return;
    }

    ret.Add("data", rm->data);
    m_csCtrl.response(rm->json, ret);   

    delete rm;
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

int
MachineServer::move_disc2cdrom(int cd_addr)
{
    char status_name[100]   = {0};
    int  cdrom_addr         = 0;

    //查找空闲光驱
    for (int i=0; i<RECORDERMAX; i++)
    {
        if (m_storage.record_attr[i].is_pluging && (!m_storage.record_attr[i].is_busy)) 
        //光驱在线,并且不忙
        {
            bool 
            isrok = (m_storage.record_attr[i].media_address == 0) || (m_storage.record_attr[i].media_address >= MIDAS_ELMADR_DT);
            isrok = isrok && (m_storage.record_attr[i].is_have_media == NOPRESENCE);
            if (!isrok){continue;}

            int status = check_cdrom_status(const_cast<char*>(m_storage.record_attr[i].dev_name));
            trans_cdrom_status(status,status_name);
            CRCLog_Info("move_disc2recorder status of %s is %s, %d", m_storage.record_attr[i].dev_name, status_name, status);
			if(CDS_TRAY_OPEN == status || CDS_NO_DISC == status){
				cdrom_addr = i + MIDAS_ELMADR_DT;
                break;
			} else {
                continue;
            }
        }
    }

    if (cdrom_addr <= 0){
        return cdrom_addr;
    }

    //找到空闲光驱
    CRCLog_Info("move_disc2recorder Find empty cdrom 0x%04x may be use for disc %d\n", cdrom_addr, cd_addr);

    int ret = move_disc2eqpt(cdrom_addr, cd_addr);

    if (ret == 0)
    {
        CRCLog_Info("move_disc2recorder Move disc %d to cdrom %d ok\n", cd_addr, cdrom_addr);
        for (int j=0;j<RECORDERMAX;j++)
        {		
            if (m_storage.record_attr[j].record_address == cdrom_addr)
            {
                m_storage.record_attr[j].is_have_media = PRESENCE;
                m_storage.record_attr[j].media_address = cd_addr;
                m_storage.record_attr[j].is_busy = 1;
                m_storage.record_attr[j].lock_num++;
                /* 光盘放时间 */
                time((time_t*)&(m_storage.record_attr[j].times)); 
                CRCLog_Info("move_disc2recorder 0x%04x,cdsrc = %04d,locknum  = %d \n",cdrom_addr,cd_addr,m_storage.record_attr[j].lock_num);		
                break;
            }		
        }

        int status = check_cdrom_status(const_cast<char*>(m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].dev_name));
        CRCLog_Info("move_disc2recorder status of %s is %d\n", m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].dev_name, status);
        if(CDS_TRAY_OPEN == status){
            cdrom_in(cdrom_addr);											
            status = check_cdrom_status(const_cast<char*>(m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].dev_name));
            CRCLog_Info("move_disc2recorder status of %s is %d\n", m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].dev_name, status);
        }
    }
    else
    {
        CRCLog_Info("move_disc2recorder Move disc %d to %d failed, %d\n", cd_addr, cdrom_addr, ret);
        if (ret == 21){
            cdrom_addr = NODISC;
        }
    }

    return cdrom_addr;
}

int
MachineServer::move_auto_disc2cdrom(CRCJson * pJson)
{
    int  try_num = 1;
    int  ret = -1;
    int  cd_addr = 0;
    int  cdrom_addr = 0;
    char cmd_buf[256];
    char label[128];
    char fstype[128];
    CRCJson retJson;

	memset(label,0,sizeof(label));
	memset(fstype,0,sizeof(fstype));

    CRCLog_Info("START move_disc2cdrom auto");

    (*pJson).Get("cdAddr",cd_addr);
    
    const char* ios_name = (*pJson)("iosName").c_str();
    const char* cd_type  = (*pJson)("cdType").c_str();

	if (access(ios_name,F_OK)!=0)
	{
		CRCLog::Error("%s is not exist!\n",ios_name);
        retJson.Add("state", STATE_CODE_ERROR);
        retJson.Add("date",  "iso name is not exist");
		goto EXIT;
	}

    //验证是否是ISO文件
    if (SVRUTILS::GetFsTypeLabel((char*)(ios_name),fstype,label))  {
        CRCLog::Error("%s is not valid iso file !\n",ios_name);
        retJson.Add("state", STATE_CODE_ERROR);
        retJson.Add("date",  "iso valid iso file");
        goto EXIT;
    }

    cd_addr = get_blank_disc(cd_addr, (char*)cd_type, SVRUTILS::GetFileSize(ios_name));
	if (cd_addr<0)
	{
		CRCLog::Error("move_disc2cdrom :cann't find blank disc %s! cd_addr:%d \n",cd_type, cd_addr);
        retJson.Add("state", STATE_CODE_ERROR);
        retJson.Add("date",  "cann't find blank disc");
		goto EXIT;
	}
    cd_addr += 1;

    MOVEDISCTOCDROM:
    {
        //为整个动作枷锁
        std::unique_lock<std::mutex> lock(m_action_mtx);            
        cdrom_addr = move_disc2cdrom(cd_addr);
    }
    if (cdrom_addr == 0){
        //如果没有找到可用的光驱，睡一会儿再试；
        CRCLog_Warring("----WAITTING FOR AVAILABLE FREE CDROM FOR %d",cd_addr);
        CRCThread::Sleep(1000*2);
        goto MOVEDISCTOCDROM;
    }

    if (cdrom_addr == NODISC){
        //delete mirror fs
        sprintf(cmd_buf,"rm -Rf %s/%04d_*",SVRUTILS::LOGPATH,cd_addr);
        SVRUTILS::SystemExec(cmd_buf);
    }

    CRCLog_Info("END process move_disc2cdrom, move disc:%d to cdrom:%d",cd_addr, cdrom_addr);    

    retJson.Add("state", STATE_CODE_OK);

    EXIT:
    m_csCtrl.response(*pJson, retJson);    

    //释放
    delete pJson;

    m_current_state = RUN;
}

int 
MachineServer::get_blank_disc(int cd_addr, char*mediatype, unsigned long long minSize, bool isSet)
{
	int i,ret  = -1;
	unsigned long long free_size  =0;
	
	char media_id[256];
	SVRUTILS::SplitMediaTypeId(mediatype, media_id);
	
	if (!strncmp(mediatype,"BD128",5))
	{
		free_size = 128001769472;
	}
    else if (!strncmp(mediatype,"BD100",5))
	{
		free_size = 100103356416;
	}
    else if (!strncmp(mediatype,"BD50",4))
	{
		free_size = 50050629632;
	}
    else if (!strncmp(mediatype,"BD25",4))
	{
		free_size = 25025314816;
	}
    else if (!strncmp(mediatype,"DVD",3))
	{
		free_size = 4706074624;
	}
    else if (!strncmp(mediatype,"CDROM",5))
	{
		free_size = 715992000; 
	}
	printf("get_blank_disc:mediatype=%s,media_id=%s\n",mediatype,media_id); 
	
	if (minSize == 0){
		minSize = free_size;
	}
	
    std::unique_lock<std::mutex> lock(m_set_mtx);

	//查找可用空盘
	if ((cd_addr >0)&&(cd_addr<MAXSTATIONMEDIA))
	{
	
		if (m_storage.media_attr[cd_addr-1].cdexist == PRESENCE)
		{
			if ((m_storage.media_attr[cd_addr-1].isblank == 1))
			{
				//dvd have multi size in china ,blu-ray is standard
				if (labs(m_storage.media_attr[cd_addr-1].diskTypeInfo.media_track_freesize - free_size)< 1024*1024*20 
					|| 
                    (is_blank_BD_RE((SDiskTypeInfo*)&m_storage.media_attr[cd_addr-1].diskTypeInfo) && m_storage.media_attr[cd_addr-1].diskTypeInfo.media_track_freesize>= (1024*1024*20 + minSize)  ))
				{
					if (isSet)
					{
						m_storage.media_attr[cd_addr-1].isblank = 0;
					}
					ret = cd_addr-1;
				}
				
			}
		}
	
	}else{

		for (i=0;i<MAXSTATIONMEDIA;i++)
		{
			if (m_storage.media_attr[i].cdexist == PRESENCE)
			{
				CRCLog::Debug("%d, BD-RE:%d, blank:%d, %s,%ld, %ld\n"
					, i+1
					, is_blank_BD_RE((SDiskTypeInfo*)&m_storage.media_attr[i].diskTypeInfo)
					, m_storage.media_attr[i].isblank
					, m_storage.media_attr[i].diskTypeInfo.media_type_str
					, m_storage.media_attr[i].diskTypeInfo.media_track_freesize
					, minSize);

				if (m_storage.media_attr[i].isblank == 1)
				{
					if (m_storage.media_attr[i].slot_status != 'N'){
						CRCLog::Info("slot_status of %d is not 'N' but '%c'\n", i, m_storage.media_attr[i].slot_status);
						continue;
					}

					CRCLog::Info("i=%d,diskTypeInfo.media_id=%s\n",i,m_storage.media_attr[i].diskTypeInfo.media_id);
					
					if (strlen(media_id)>0 && strncmp(const_cast<char*>(m_storage.media_attr[i].diskTypeInfo.media_id), media_id, strlen(media_id)) != 0){
						continue;
					}

			        CRCLog::Debug("%d, %s, BD-RE:%d, blank:%d, %s,%lld, %lld\n"
                        , i+1
					    , const_cast<char*>(m_storage.media_attr[i].diskTypeInfo.media_id)
                        , is_blank_BD_RE((SDiskTypeInfo*)&m_storage.media_attr[i].diskTypeInfo)
                        , m_storage.media_attr[i].isblank
                        , m_storage.media_attr[i].diskTypeInfo.media_type_str
                        , m_storage.media_attr[i].diskTypeInfo.media_track_freesize
                        , minSize);

					CRCLog::Info("i=%d,media_track_freesize=%llu,free_size=%llu,minSize=%llu,media_track_freesize - free_siz=%ld,(1024*1024*20 + minSize)=%llu\n",i,m_storage.media_attr[i].diskTypeInfo.media_track_freesize,free_size,minSize,labs(m_storage.media_attr[i].diskTypeInfo.media_track_freesize - free_size),(1024*1024*20 + minSize));  //20180612
					CRCLog::Info("is_blank_BD_RE(&m_storage.media_attr[i].diskTypeInfo)=%d\n",is_blank_BD_RE((SDiskTypeInfo*)&m_storage.media_attr[i].diskTypeInfo));
					
					if (
                        labs(m_storage.media_attr[i].diskTypeInfo.media_track_freesize - free_size)< 1024*1024*20 
					    || 
                        (is_blank_BD_RE((SDiskTypeInfo*)&m_storage.media_attr[i].diskTypeInfo) && m_storage.media_attr[i].diskTypeInfo.media_track_freesize >= (1024*1024*20 + minSize) ))
					{
						if (isSet)
						{
							m_storage.media_attr[i].isblank = 0;
						}
						ret = i;
						break;
					}	
					
				}
			}
		}
		
	}

    lock.unlock();

	if(ret < 0){
        CRCLog::Warring("no blank disc to burn %s,%lld", mediatype, minSize);
	}

    return ret;
}

int 
MachineServer::check_cdrom_status(const char * cdrom_dev)
{
	int cdrom;
	int status=-1;

    //暂时测试用
    if (true){return CDS_TRAY_OPEN;}

	if (cdrom_dev == NULL){
		return -1;
	}

    //补全光驱设备名
	char full_name[256];
	if(cdrom_dev[0] != '/'){
		sprintf(full_name,"/dev/%s", cdrom_dev);
	}
	else{
		strcpy(full_name, cdrom_dev);
	}

    //打开设备文件
	if ((cdrom = open(full_name,O_RDONLY | O_NONBLOCK)) < 0) {
		CRCLog_Error("Unable to open device %s. Provide a device name (/dev/sr0, /dev/cdrom) as a parameter.\n",full_name);
		return -1;
	}

	//获取光驱状态
	status = ioctl(cdrom,CDROM_DRIVE_STATUS);

    //关闭文件描述符
	close(cdrom);
	
	CRCLog_Info("check_cdrom_status:return status=%d\n",status);
	
	return status;

}

void
MachineServer::trans_cdrom_status(int status, char* status_name)
{
	memset(status_name,0,100);
	switch(status)
	{
	case CDS_NO_INFO:
		strcpy(status_name, "CDS_NO_INFO");
		break;
	case CDS_NO_DISC:
		strcpy(status_name, "CDS_NO_DISC");
		break;
	case CDS_TRAY_OPEN:
		strcpy(status_name, "CDS_TRAY_OPEN");
		break;
	case CDS_DRIVE_NOT_READY:
		strcpy(status_name, "CDS_DRIVE_NOT_READY");
		break;
	case CDS_DISC_OK:
		strcpy(status_name, "CDS_DISC_OK");
		break;
	default:
		sprintf(status_name, "UNKNOWN %d",status);
	}    
}

int 
MachineServer::move_disc2eqpt(int dest_addr, int src_addr, std::string * errStr)
{
    //定义命令
    std::string from        = "move_disc2eqpt";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;
    CRCJson *   pMsg        = new CRCJson();

    //组装命令
    char cmd_buff[256];
    sprintf(cmd_buff,"CMOV,0x%04x,0x%04x,\n",dest_addr,src_addr);
    CRCLog_Info("move_disc2recorder command: %s\n", cmd_buff);    

    //生成任务对象
    pMsg->Add("cmdTransfer",    cmd_buff);
    pMsg->Add("from",           from);
    pMsg->Add("fromId",         fromId);
    pMsg->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pMsg);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pMsg);
    }

    //等待结果返回
    CRCLog_Info("move_disc2eqpt WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "move_disc2eqpt recv timeout", rm))
    {
        if (errStr){
            errStr->append("move_disc2eqpt recv timeout"); 
        }
        delete pMsg;
        return -999;
    }

    int ret = -1;

    if (strncmp(rm->data,"RET,0",5) == 0)
    {
        ret = 0;
        if(src_addr == MIDAS_ELMADR_PR){
            SvrPrinter::Inst().lp()->media_address = MIDAS_ELMADR_PR;
            CRCLog_Info("move_disc2eqpt return disc from printer to %d ok!", dest_addr);
        }
    }
    //dests have disc
    else if (strncmp(rm->data,"RET,31",6) == 0)
    {
        CRCLog_Info("move_disc2eqpt dests %d has disc!",dest_addr);
        if(src_addr == MIDAS_ELMADR_PR){
            SvrPrinter::Inst().lp()->media_address = MIDAS_ELMADR_PR;
            CRCLog_Info("move_disc2eqpt return disc by at 91 from printer to %d ok!", dest_addr);
        }
        ret = 31;
    }
    //src have no disc
    else if (strncmp(rm->data,"RET,21",6) == 0)
    {
        CRCLog_Info("move_disc2eqpt src %d has no disc!", src_addr);
        ret = 21;
    }
    //carrier fail
    else if (strncmp(rm->data,"RET,92",6) == 0)
    {
        CRCLog_Info("move_disc2eqpt carrier fail (%d->%d)!", src_addr, dest_addr);
        ret = 92;
    }
    //addr is no valid
    else if (strncmp(rm->data,"RET,10",6) == 0)
    {
        CRCLog_Info("move_disc2eqpt addr is no valid or busy (%d->%d)!", src_addr, dest_addr);
        ret = 10;
    }
    //unknow error
    else
    {
        CRCLog_Info("move_disc2eqpt unknown error (%d->%d)!", src_addr, dest_addr);
        ret = -1;
    }
	if (dest_addr == MIDAS_ELMADR_PR)
	{
		if (ret == 0)
		{
			SvrPrinter::Inst().lp()->is_have_media = 1;
			SvrPrinter::Inst().lp()->media_address = src_addr;
			CRCLog_Info("move_disc2eqpt move %d into printer ok!", src_addr);
		}
		else{
			CRCLog_Error("move_disc2eqpt move %d into printer failed!", src_addr);
		}
	}
    if (errStr){errStr->append(rm->data);}
    if (rm) {delete rm;}
    
    return ret;
}

int 
MachineServer::cdrom_in(int cdrom_addr)
{
    //定义命令
    std::string from        = "cdrom_in";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;
    CRCJson *   pMsg        = new CRCJson();

    //组装命令
    char cmd_buff[256];
    sprintf(cmd_buff,"CDIN,0x%04x,\n",cdrom_addr);
    CRCLog_Info("cdrom_in command: %s\n", cmd_buff);    

    //生成任务对象
    pMsg->Add("cmdTransfer",    cmd_buff);
    pMsg->Add("from",           from);
    pMsg->Add("fromId",         fromId);
    pMsg->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pMsg);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pMsg);
    }

    //等待结果返回
    CRCLog_Info("cdrom_in WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "cdrom_in recv timeout", rm))
    {
        delete pMsg;
        return -999;
    }

    int ret;
    if (strncmp(rm->data,"RET,0",5) == 0)
    {
        ret = 0;
    }else{
        CRCLog_Error("CDROM IN failed! msg: %s\n", rm->data);
        ret = -1;
    }

    if (rm) {delete rm;}

    return ret;
}

int
MachineServer::check_cd_cdrom(int cd_addr, int cdrom_addr, int md5_check)
{
	int             ret = -1;
	char            dev_name[64];
	char            cmd_buf[256];
	char            mount_point[256];
	char            mod_cmd[512];
    SDiskTypeInfo   diskTypeInfo;

    memset(&diskTypeInfo,0,sizeof(diskTypeInfo));

    snprintf(cmd_buf,sizeof(cmd_buf),"rm -Rf %s/%04d_*",SVRUTILS::MIRRORPATH,cd_addr);
    SVRUTILS::SystemExec(cmd_buf);
    trans_addr2devname(cdrom_addr,dev_name);

    if (cdrom_is_ready(dev_name))
    {
        CRCLog_Error("check_cd_cdrom:CDROM is not ready! %s, %d, %d\n", dev_name, cdrom_addr, cd_addr);

        SVRUTILS::DISC_CHECK_BAD_TIMES[cd_addr-1]++;

		if (SVRUTILS::DISC_CHECK_BAD_TIMES[cd_addr-1]==3)
		{
		    CRCLog_Error("		getdisc fail 3 times\n");
		    snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_BadMedia",SVRUTILS::MIRRORPATH,cd_addr);
			if (access(cmd_buf,F_OK)!=0)
			{
				if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}
			snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			CRCLog_Error("%d	 is  damage disc\n",cd_addr);
			/* 0 unknown ,1 blank 2 damage disk */
			m_storage.media_attr[cd_addr-1].isblank     = 2;
			m_storage.media_attr[cd_addr-1].ischecked   = 1;
			return cd_addr;
		}
		return NOINFO; 
    }

    //如果盘读不出来，进出一次再读
	if (GetMediaInfo(dev_name,&diskTypeInfo))
	{
		CRCLog_Error("		check_cd_cdrom:DISK NO INFO!\n");
		SVRUTILS::DISC_CHECK_BAD_TIMES[cd_addr-1]++;
		if(SVRUTILS::DISC_CHECK_BAD_TIMES[cd_addr-1] == 3)
		{
		    CRCLog_Error("		getdiskfail 3\n");
		    snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_BadMedia",SVRUTILS::MIRRORPATH,cd_addr);
			if (access(cmd_buf,F_OK)!=0)
			{
				if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}
			snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			CRCLog_Error("%d	 is  damage disk\n",cd_addr);
			/* 0 unknown ,1 blank 2 damage disk */
			m_storage.media_attr[cd_addr-1].isblank     = 2;
			m_storage.media_attr[cd_addr-1].ischecked   = 1;
			return cd_addr;
		}
		return NOINFO;   
	}

    CRCLog_Debug("%d after GetMediaInfo(dev_name,&diskTypeInfo)\n",__LINE__);

	ret  =  cd_addr;

	//remove old grep info
	{
		char rm_cmd[1024];
	    CRCLog_Info("serial num %s\n",m_storage.mag_slotarray[(cd_addr-1)/50].magazine.serial);
		snprintf(rm_cmd,sizeof(rm_cmd),"rm -f %s/%04d-*.grep",SVRUTILS::DINFOPATH,cd_addr);
		SVRUTILS::SystemExec(rm_cmd);
	}
	printf("disk info of %d, id:%s, fs:%s, type:%s, status:%s, session:%s, session_number:%d, tracks:%d, capacity:%llu, freesize:%llu,label:%s\n", 
        cd_addr,                                    //int cd_addr
        diskTypeInfo.media_id,                      //char media_id[32]
        diskTypeInfo.fs_type,                       //char fs_type[16]
        diskTypeInfo.media_type_str,                //char media_type_str[32]
        diskTypeInfo.media_status_str,              //char media_status_str[16]
        diskTypeInfo.media_last_session_status,     //char media_last_session_status[16]
        diskTypeInfo.media_number_session,          //char media_number_session
        diskTypeInfo.media_number_tracks,           //char media_number_tracks
        diskTypeInfo.media_capacity,                //unsigned long long media_capacity
        diskTypeInfo.media_track_freesize,          //unsigned long long media_track_freesize
        diskTypeInfo.label_name                     //char label_name[128]
    );

	memcpy((void*)&m_storage.media_attr[cd_addr-1].diskTypeInfo, &diskTypeInfo, sizeof(diskTypeInfo));

	m_storage.media_attr[cd_addr-1].media_addr      = cd_addr;
	m_storage.media_attr[cd_addr-1].isblank         = 0;
	m_storage.media_attr[cd_addr-1].cdexist         = PRESENCE;
	m_storage.media_attr[cd_addr-1].trayexist       = PRESENCE;

	if (!strcmp(diskTypeInfo.media_status_str,"appendable"))
	{
		m_storage.media_attr[cd_addr-1].isappendable = 1;
	}

    //mount point
    snprintf(mount_point,sizeof(mount_point),"/tmp/%04d",cd_addr);
    SVRUTILS::CheckDirIsExist(mount_point);

    //有文件系统，如果是多段的话，必须最后一段是empty,否则是坏盘
    if  (   (   
                (strlen(diskTypeInfo.fs_type))||
                (
                    (!strcmp(diskTypeInfo.media_last_session_status,"empty"))&&
                    (!strcmp(diskTypeInfo.media_status_str,"appendable"))&&
                    (diskTypeInfo.media_number_session >1) 
                )
            )&&
            (strcmp(diskTypeInfo.media_last_session_status,"incomplete"))
        )
    {
        if (xmount(dev_name,mount_point,diskTypeInfo.fs_type,cdrom_addr) == 0)
        {
            memset(cmd_buf,0,sizeof(cmd_buf));

			if (strlen(diskTypeInfo.label_name) > 0)
			{
				snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_%s",SVRUTILS::MIRRORPATH,cd_addr,diskTypeInfo.label_name);
			}else{
				snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_%s",SVRUTILS::MIRRORPATH,cd_addr,diskTypeInfo.media_type_str);
			}

			if (access(cmd_buf,F_OK)!=0)
			{
				if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}

			CRCLog_Info("check_cd_cdrom: StorageDupDir %s\n",cmd_buf);

            if(SvrDir::Inst().DuplicateDir(mount_point,cmd_buf) != 0){
                CRCLog_Error("check_cd_cdrom: duplicate dir %s from %s failed!\n", cmd_buf, mount_point);
                return DUPDIRERR;
            } else {
				snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
				SVRUTILS::SystemExec(mod_cmd);
				snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
				SVRUTILS::SystemExec(mod_cmd);

                //generate grep info
				{
					char grep_cmd[1024];
					snprintf(grep_cmd,sizeof(grep_cmd),"find \"%s\" >%s/%04d-%s.grep",
						cmd_buf,SVRUTILS::DINFOPATH,cd_addr,
						m_storage.mag_slotarray[(cd_addr-1)/50].magazine.serial);
					SVRUTILS::SystemExec(grep_cmd);
				}

                if(md5_check)
                {
					int ret_check = SVRUTILS::CheckMd5(mount_point);
					if (ret_check == 2)
                    {
						CRCLog_Error("check_cd_cdrom: MD5 check failed: %d\n", ret_check);

						snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_BadMedia",SVRUTILS::MIRRORPATH,cd_addr);

						if (access(cmd_buf,F_OK)!=0)
						{
							if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
							{
								perror("mkdir:");
								return -1;
							}
						}
						snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
						SVRUTILS::SystemExec(mod_cmd);
						snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
						SVRUTILS::SystemExec(mod_cmd);
						CRCLog_Error("check_cd_cdrom: %d   is  DAMAGE disk\n",cd_addr);
						/* 0 unknown ,1 blank 2 damage disk */
						m_storage.media_attr[cd_addr-1].isblank = 2;
                    }

                }//!if(md5_check)

            }//!if(SvrDir::Inst().DuplicateDir(mount_point,cmd_buf) != 0)::else

        }//!if (xmount(dev_name,mount_point,diskTypeInfo.fs_type,cdrom_addr)
        else 
        //光盘信息可以读出来，但是无法mount上，可能是光盘坏了，或是光驱的问题，标记为坏盘
        {
			sprintf(cmd_buf,"%s/%04d_BadMedia",SVRUTILS::MIRRORPATH,cd_addr);
			if (access(cmd_buf,F_OK)!=0)
			{
				if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}
			snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
        }//!if (xmount(dev_name,mount_point,diskTypeInfo.fs_type,cdrom_addr)::else

    }//!有文件系统，如果是多段的话，必须最后一段是empty,否则是坏盘
    else 
    //空盘 ,多轨盘空盘，格式化过的空盘
    {
        if ((!strcmp(diskTypeInfo.media_status_str,"blank"))|| is_blank_BD_RE(&diskTypeInfo) )
        {
			m_storage.media_attr[cd_addr-1].isblank = 1;
			snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_Blank_%s",SVRUTILS::MIRRORPATH,cd_addr,diskTypeInfo.media_type_str);
			if (access(cmd_buf,F_OK)!=0)
			{
				if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}
			snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			//设定全盘描述
			m_storage.media_attr[cd_addr-1].media_addr  = cd_addr;
			m_storage.media_attr[cd_addr-1].isblank     = 1;
			CRCLog_Info("check_cd_cdrom: %d is  blank disk, %s  \n",cd_addr, diskTypeInfo.media_status_str);
        }//!if ((!strcmp(diskTypeInfo.media_status_str,"blank"))|| is_blank_BD_RE(&diskTypeInfo) )
        else if (
                    (!strcmp(diskTypeInfo.media_last_session_status,"empty"))&&(!strcmp(diskTypeInfo.media_status_str,"appendable"))
                )
        {
			m_storage.media_attr[cd_addr-1].isblank = 0;
			m_storage.media_attr[cd_addr-1].isappendable = 1;
			snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_AppenBlank",SVRUTILS::MIRRORPATH,cd_addr);
			if (access(cmd_buf,F_OK)!=0)
			{
				if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}
			snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			/* 设定全盘描述 */
			m_storage.media_attr[cd_addr-1].media_addr = cd_addr;
			CRCLog_Info("check_cd_cdrom: %d is  appendable blank disk  \n",cd_addr);
        }//!if ((!strcmp(diskTypeInfo.media_status_str,"blank"))|| is_blank_BD_RE(&diskTypeInfo) )::else if
        else 
        {
			snprintf(cmd_buf,sizeof(cmd_buf),"%s/%04d_BadMedia",SVRUTILS::MIRRORPATH,cd_addr);
			if (access(cmd_buf,F_OK)!=0)
			{
				if (mkdir(cmd_buf,S_IRWXU|S_IWOTH|S_IRWXG|S_IRGRP|S_IWOTH) !=0)
				{
					perror("mkdir:");
					return -1;
				}
			}
			snprintf(mod_cmd,sizeof(mod_cmd),"chown -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			snprintf(mod_cmd,sizeof(mod_cmd),"chgrp -Rf admin \"%s\"",cmd_buf);
			SVRUTILS::SystemExec(mod_cmd);
			CRCLog_Info("check_cd_cdrom: %d   is  DAMAGE disk\n",cd_addr);
			/* 0 unknown ,1 blank 2 damage disk */
			m_storage.media_attr[cd_addr-1].isblank = 2;
        }//!if ((!strcmp(diskTypeInfo.media_status_str,"blank"))|| is_blank_BD_RE(&diskTypeInfo) )::else
    }

    m_storage.media_attr[cd_addr-1].ischecked = 1;

    return ret;
}

int
MachineServer::trans_addr2devname(int addr,char *dev_name)
{
	int ret = -1;

	for (int i = 0;i < RECORDERMAX; i++ )
	{
		/* 排除没有连接的光驱 */
		if (m_storage.record_attr[i].is_pluging !=1)
		{
			continue;
		}
		if (m_storage.record_attr[i].record_address == addr)
		{
			if (dev_name)
			{
				memset(dev_name,0,32);
				strncpy(dev_name, const_cast<char*>(m_storage.record_attr[i].dev_name),31);
				ret = 0;
				break;
			}
		}
		
	}
	return ret ;    
}

int
MachineServer::cdrom_is_ready(const char* cdrom_dev)
{
	int status;
	int ret = 0;
	int try_times = 0;

	CRCLog_Info("Wait %s Ready !\n",cdrom_dev);
    
    if (!cdrom_dev){
        return -1;
    }

	for(;;)
	{
		status =  check_cdrom_status(cdrom_dev);
		CRCThread::Sleep(1000);
		switch (status)
		{
		/* media maybe bad */
		case CDS_NO_INFO:
			CRCLog_Info("		%s CDS_NOINFO\n",cdrom_dev);
			{
				ret = -1;
				return ret;
			}

		case CDS_NO_DISC:
			CRCLog_Info("		%s CDS_NO_DISC \n",cdrom_dev);
			{
				ret = -1;
				return ret;
			}
			/* wait tray close */
		case CDS_TRAY_OPEN:
			CRCLog_Info("		%s,%d,CDS_TRAY_OPEN \n",cdrom_dev,try_times);
			{
				if(try_times >= 5){
					ret = -1;
					return ret;
				}
				try_times++;
				break;
			}
		case CDS_DRIVE_NOT_READY:
			CRCLog_Info("		%s,CDS_DRIVE_NOT_READY \n",cdrom_dev);
			CRCThread::Sleep(2);
			break;
		case CDS_DISC_OK:
			CRCLog_Info("		%s,CDS_DISC_OK \n",cdrom_dev);
			ret = 0;
			return ret;
		case -1:
			CRCLog_Info("cdrom failure,errno %d \n",errno);
			CRCThread::Sleep(2);
			return -1;
		default:
			CRCLog_Info("Unknown status %d\n", status);
            return -1;
			break;
		}
		
	}    
}

int 
MachineServer::xmount(char *devname, char* mntdir, char *fstype, int cdrom_addr)
{
	char cmd_buff[256];

	SVRUTILS::CheckDirIsExist(mntdir);

	//检查是设备是否MOUNT到指定路径上
	if (xumount(devname,0,cdrom_addr) == -1)
	{
		memset(cmd_buff,0,sizeof(cmd_buff));
		if (strlen(fstype))
		{
			sprintf(cmd_buff,"mount -t %s   %s %s",fstype,devname,mntdir);
		}else{
			sprintf(cmd_buff,"mount -t auto   %s %s",devname,mntdir);
		}
		CRCLog::Info("%s \n",cmd_buff);
		if (SVRUTILS::SystemExec(cmd_buff))
		{
			return -1;
		}
		if( (cdrom_addr > MIDAS_ELMADR_DT) && (cdrom_addr < MIDAS_ELMADR_DT+RECORDERMAX) )
		{
			m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].is_mount = 1;
			if (strlen(mntdir) >0)
			{
				memset ((void*)m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].mnt_dir,0,256); 
				strncpy((char*)m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].mnt_dir,mntdir,255); 
			}
			
		}
		return xumount(devname,0,cdrom_addr);	
	}else{
		return 0;
	}
}

int
MachineServer::xumount(char *devname,int flag,int cdrom_addr)
{
	struct mntent mnt;
	FILE *aFile=NULL;
	int  ret =-1;
	char cmd_buff[256];
    char mnt_buf[1024];

    memset(&mnt,0x00,sizeof(mnt));
    memset(&mnt_buf[0],0x00,sizeof(mnt_buf));

	aFile = setmntent("/proc/mounts", "r");  //利用/proc/mounts检查已经被系统挂载的设备

	if (aFile == NULL) {
		CRCLog_Error("open mnt list failed for %s\n",devname);
		perror("setmntent");
		return -1;
	}

    //这里不使用更简单的 getmntent，因为它不可重入
	while (NULL != getmntent_r(aFile, &mnt, mnt_buf, sizeof(mnt_buf)) ) 
    {
		//两次mount 在不同位置上 全部UMOUNT
		if (strcmp(devname,mnt.mnt_fsname) == 0)
		{
		    CRCLog_Info("xumount %s %s\n", mnt.mnt_fsname, mnt.mnt_dir);

			if (flag)
			{
				CRCLog_Info("umount  %s %s 0x%04x\n",devname, mnt.mnt_dir, cdrom_addr);
				//MNT_EXPIRE (since Linux 2.6.8)
				sprintf(cmd_buff,"umount -f %s",devname);
				if (SVRUTILS::SystemExec(cmd_buff))
				{
					CRCLog_Error("umount  %s %s failed\n",devname,mnt.mnt_dir);
					ret = -1;
				}else{
					if ( (cdrom_addr >= MIDAS_ELMADR_DT) && (cdrom_addr < MIDAS_ELMADR_DT+RECORDERMAX) )
					{
						m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].is_mount = 0;
						memset(
                            (void*)m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].mnt_dir,
                            0,
							sizeof(m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].mnt_dir));
						CRCLog_Info("update mount flag to 0 of 0x%04x\n", cdrom_addr);
					}
					else{
						CRCLog_Error("invalid record_addr 0x%04x to update mount flags\n", cdrom_addr);
					}
					ret =0;
				}
			}else{
				ret = 0;
            }
		}
	}

	if(aFile){ 
		endmntent(aFile);
		aFile=NULL; 
	}
	return ret ;    
}

int 
MachineServer::is_blank_BD_RE(SDiskTypeInfo* info)
{
    CRCLog_Debug("%s,%s,%s,%d\n",info->media_type_str,info->media_last_session_status,info->media_status_str,info->media_number_session);

	if(strcmp(info->media_type_str, "BD-RE") != 0){
		return 0;
	}

	return !strcmp(info->media_last_session_status,"complete")
		&& !strcmp(info->media_status_str,"complete")
		&& info->media_number_session == 1;    
}

int 
MachineServer::cdrom_return_disc(int cdrom_addr)
{
    int  ret            = -1;
    char cmd_buff[256]  = {0};

    /*
    sprintf(cmd_buff,"CDOU,0x%04x,\n",cdrom_addr);
    CRCLog_Info("cdrom_return_disc command: %s\n", cmd_buff);    

    for (int i=0; i<3; i++)
    {
        //定义命令
        std::string from        = "cdrom_return_disc";
        uint32_t    fromId      = m_atomic++;
        RetMessage* rm          = nullptr;
        CRCJson *   pMsg        = new CRCJson();

        //生成任务对象
        pMsg->Add("cmdTransfer",    cmd_buff);
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
        CRCLog_Info("cdrom_return_disc WAITTING CDOU msg key<%s> return ...", key.c_str());

        if (!wait_for(key, "cdrom_return_disc recv timeout", rm))
        {
            return -999;
        }

        if (strncmp(rm->data,"RET,0",5) == 0){
            ret = 0;
            delete rm;
            break;
        } else {
            delete rm;
        }
    }

    if (ret != 0){
        return ret;
    }*/

    //组装命令
    memset (cmd_buff,0,sizeof(cmd_buff));
    sprintf(cmd_buff,"HIDE,0x%04x,\n",cdrom_addr);

    int cd_addr = m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].media_address;

    std::string from        = "cdrom_return_disc";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;
    CRCJson *   pMsg        = new CRCJson();

    //生成任务对象
    pMsg->Add("cmdTransfer",    cmd_buff);
    pMsg->Add("from",           from);
    pMsg->Add("fromId",         fromId);
    pMsg->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pMsg);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pMsg);
    }

    //等待结果返回
    CRCLog_Info("cdrom_return_disc WAITTING HIDE msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "cdrom_return_disc recv timeout", rm))
    {   
        delete pMsg;
        return -999;
    }

    if (strncmp(rm->data,"RET,0",5) == 0)
    //回盘成功
    {

        printf("cdrom_return_disc:dev: 0x%04x to %d, succesful!\n",cdrom_addr, cd_addr);
        //更新光驱信息
        m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].is_have_media = NOPRESENCE;
        m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].media_address = cdrom_addr;
        m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].is_busy       = 0;
        m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].lock_num      = 0;
        m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].is_mount      = 0;
        //更新光盘信息
        m_storage.media_attr[cd_addr-1].trayexist   = PRESENCE;	
        m_storage.media_attr[cd_addr-1].cdexist     = PRESENCE;
        ret = 0;
    }
    //回盘失败
    else
    {
        if (strncmp(rm->data,"RET,-2",6) == 0)
        //光驱里本来就没盘
        {
            printf("cdrom_return_disc:dev: 0x%04x to %d, src have no disc!",cdrom_addr, cd_addr);
            m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].is_have_media = NOPRESENCE;
            m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].media_address = cdrom_addr;
            m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].is_busy       = 0;
            m_storage.record_attr[cdrom_addr-MIDAS_ELMADR_DT].lock_num      = 0;
            ret = -2;
        }
        else
        {
            ret = -1;
        }
        
    }
    
    delete rm;
    return ret;
}

int 
MachineServer::cdrom_return_disc()
{
	for (int i =0; i < RECORDERMAX; i++)
	{
		if (m_storage.record_attr[i].is_pluging && m_storage.record_attr[i].is_have_media == PRESENCE)
		{
			int ret = cdrom_return_disc(MIDAS_ELMADR_DT+i);
            if (ret != 0){break;}
		}
		
	}    
}

int 
MachineServer::query_station(CRCJson* pJson)
{
    std::string data;
    CRCJson jdata;
    char media_type[100];
    char media_type_str[101];

    /*
    data
        .append("{")
        .append("\"MachineType\":\"").append(const_cast<char*>(m_storage.midas_box.midas_attr.box_name)).append("\",")
        .append("\"DoorStatus\":").append(1, m_storage.midas_box.door.door_flag).append(",")
        .append("\"EventCnt\":").append("1").append(",")
        .append("\"Mag\":[");*/

    jdata.Add("MachineType",const_cast<char*>(m_storage.midas_box.midas_attr.box_name));
    jdata.Add("DoorStatus",m_storage.midas_box.door.door_flag);
    jdata.Add("EventCnt",1);
    jdata.AddEmptySubArray("Mag");

    for (int i=0; i<12/*MAGSLOTMAX*/; i++)
    {
        if (m_storage.mag_slotarray[i].mag_plug == 0){
            /*
            data
                .append("\"MagNo\":").append(std::to_string(i)).append(",")
                .append("\"Rfid\": ,\"unknown\":[]");*/

            CRCJson tmpJson;

            tmpJson.Add("MagNo", i);
            tmpJson.Add("Rfid", "");
            tmpJson.Add("unknown", "[]");

            jdata["Mag"].Add(tmpJson);

        } else {
            /*
            data
                .append("{")
                .append("\"MagNo\":").append(std::to_string(i)).append(",")
                .append("\"Rfid\": \"").append(const_cast<char*>(m_storage.mag_slotarray[i].magazine.serial)).append("\",")
                .append("\"Slot\":[");*/

            CRCJson tmpJson;

            tmpJson.Add("MagNo", i);
            tmpJson.Add("Rfid", const_cast<char*>(m_storage.mag_slotarray[i].magazine.serial));
            tmpJson.AddEmptySubArray("Slot");

            for (int j=0; j<MAGITEMMAX; j++)
            {
                memset(media_type, 0, sizeof(media_type));
                memset(media_type_str, 0, sizeof(media_type_str));

				if (
                    strncmp(const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str),"BD-",3) == 0 &&
                    strcmp (const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_last_session_status),"empty") == 0 &&
                    strcmp (const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_status_str),"blank") == 0)
                {
					if (m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_capacity == 0){
						snprintf(
                            media_type,sizeof(media_type),"BD%.0f-%s",
                            m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_track_freesize/1000.0/1000/1000,
                            m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str+3);
                    } else {
						snprintf(
                            media_type,sizeof(media_type),"BD%.0f-%s",
							m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_capacity/1000.0/1000/1000,
							m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str+3);

                    }
                }
				else if(
                    strncmp(const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str),"BD-",3) == 0 &&
					strcmp (const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_last_session_status),"complete") == 0 &&
					strcmp (const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_status_str),"complete") == 0){
					if (m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_capacity == 0){
							snprintf(media_type,sizeof(media_type),"BD%.0f-%s",
                            m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_track_freesize/1000.0/1000/1000,
                            m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str+3);
					}
					else{
						snprintf(media_type,sizeof(media_type),"BD%.0f-%s",
							m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_capacity/1000.0/1000/1000,
							m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str+3);
					}
				}
				else{
					strncpy(media_type, const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_type_str),sizeof(media_type)-1);
				}

				if (!strcmp(const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_status_str),"appendable")&&
                    SVRUTILS::TRACK_NOT_CLOSED[i*MAGITEMMAX+j]==0){
					snprintf(media_type_str,sizeof(media_type_str),"+%s",media_type);
				}
				else{
					snprintf(media_type_str,sizeof(media_type_str),"%s",media_type);
				}

				char label_formatted1[256];
				char label_formatted2[256];

				memset(label_formatted1, 0, 256);
				memset(label_formatted2, 0, 256);

                SVRUTILS::StrRpl(const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.label_name), label_formatted1, 256, "\\", "\\\\");
                strncpy         (label_formatted2, label_formatted1,sizeof(label_formatted2)-1);
                memset          (label_formatted1, 0, 256);
                SVRUTILS::StrRpl(label_formatted2, label_formatted1, 256, "\"", "\\\"");

                /* 下面两种方法都只能拼接到 mediatype 就拼接不下去了，原因未知，可能是std::string的BUG
                data.append("{")
                    .append("\"id\":")          .append(std::to_string(i*MAGITEMMAX+j+1)).append(",")
                    .append("\"media_id\":\"")  .append(const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_id)).append("\",")
                    .append("\"cdexist\":")     .append(1, m_storage.media_attr[i*MAGITEMMAX+j].cdexist).append(",")
                    .append("\"trayexist\":")   .append(1, m_storage.media_attr[i*MAGITEMMAX+j].trayexist).append(",")
                    .append("\"ischecked\":")   .append(1, m_storage.media_attr[i*MAGITEMMAX+j].ischecked).append(",")
                    .append("\"isblank\":")     .append(1, m_storage.media_attr[i*MAGITEMMAX+j].isblank).append(",")
                    .append("\"mediatype\":\"") .append(strlen(media_type_str)==0?"none":media_type_str).append("\",")
                    .append("\"label\":\"")     .append(strlen(label_formatted1)==0?"none":label_formatted1).append("\",")
                    .append("\"slot_status\":") .append(std::to_string(m_storage.media_attr[i*MAGITEMMAX+j].slot_status))
                    .append("}");

                data +=
                    std::string("{") +
                    std::string("\"id\":")          + std::to_string(i*MAGITEMMAX+j+1) + "," +
                    std::string("\"media_id\":\"")  + const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_id) + "\"," +
                    std::string("\"cdexist\":")     + m_storage.media_attr[i*MAGITEMMAX+j].cdexist + "," +
                    std::string("\"trayexist\":")   + m_storage.media_attr[i*MAGITEMMAX+j].trayexist + "," + 
                    std::string("\"ischecked\":")   + m_storage.media_attr[i*MAGITEMMAX+j].ischecked + "," + 
                    std::string("\"isblank\":")     + m_storage.media_attr[i*MAGITEMMAX+j].isblank + "," + 
                    std::string("\"mediatype\":\"") + (strlen(media_type_str)==0?"none":media_type_str) + "\"," + 
                    std::string("\"label\":\"")     + (strlen(label_formatted1)==0?"none":label_formatted1) + "\"," + 
                    std::string("\"slot_status\":") + std::to_string(m_storage.media_attr[i*MAGITEMMAX+j].slot_status) + 
                    std::string("}");*/

                CRCJson tmpSubJson;

                tmpSubJson.Add("id",          i*MAGITEMMAX+j+1);
                tmpSubJson.Add("media_id",    const_cast<char*>(m_storage.media_attr[i*MAGITEMMAX+j].diskTypeInfo.media_id));
                tmpSubJson.Add("cdexist",     m_storage.media_attr[i*MAGITEMMAX+j].cdexist);
                tmpSubJson.Add("trayexist",   m_storage.media_attr[i*MAGITEMMAX+j].trayexist);
                tmpSubJson.Add("ischecked",   m_storage.media_attr[i*MAGITEMMAX+j].ischecked);
                tmpSubJson.Add("isblank",     m_storage.media_attr[i*MAGITEMMAX+j].isblank);
                tmpSubJson.Add("mediatype",   strlen(media_type_str)==0?"none":media_type_str);
                tmpSubJson.Add("label",       strlen(label_formatted1)==0?"none":label_formatted1);
                tmpSubJson.Add("slot_status", m_storage.media_attr[i*MAGITEMMAX+j].slot_status);

                tmpJson["Slot"].Add(tmpSubJson);

            }//!for (int j=0; j<MAGITEMMAX; j++)

            //data.append("]}");
            jdata["Mag"].Add(tmpJson);
        }

    }//!for (i=0; i<12/*MAGSLOTMAX*/; i++)

    //data.append("],");

    jdata.AddEmptySubArray("Drivers");

    for (int i = 0;i<6/*RECORDERMAX*/;i++)  //一般最多也就6个光驱
    {
		if (m_storage.record_attr[i].is_pluging)
		{
			char progress[256];
			char label[256];

			memset(progress,0,sizeof(progress));
			memset(label,0,sizeof(label));

			if (m_storage.record_attr[i].is_have_media == 'B')
			{
				char dev_name[32];
				snprintf(dev_name,sizeof(dev_name),"sr%d",i);
				SVRUTILS::GetCdromProgress(dev_name,progress);

				strncpy(label, const_cast<char*>(m_storage.record_attr[i].mnt_dir),sizeof(label)-1);   
				SVRUTILS::StrRpl(const_cast<char*>(m_storage.record_attr[i].burning_label), label, 256, "\"", "\\\"");
			}
			else{
				strcpy(progress, "");
				strcpy(label, "");
			}

            CRCJson tmpJson;

            tmpJson.Add("id",           const_cast<char*>(m_storage.record_attr[i].dev_name));
            tmpJson.Add("ishavemedia",  m_storage.record_attr[i].is_have_media);
            tmpJson.Add("cd_src",       m_storage.record_attr[i].media_address);
            tmpJson.Add("progress",     progress);
            tmpJson.Add("burning_label",label);

            jdata["Drivers"].Add(tmpJson);
		}else{

            CRCJson tmpJson;
            
            tmpJson.Add("id",           "");
            tmpJson.Add("ishavemedia",  "");
            tmpJson.Add("cd_src",       "");
            tmpJson.Add("progress",     "");
            tmpJson.Add("burning_label","");

            jdata["Drivers"].Add(tmpJson);

		}//!if (m_storage.record_attr[i].is_pluging)

    }//!for (int i = 0;i<6/*RECORDERMAX*/;i++) 

	char status_desc[1024];
	memset(status_desc, 0, 1024);
	char tmp[1024];
	memset(tmp, 0, 1024);

	strncpy(tmp, SvrPrinter::Inst().lp()->status_desc,sizeof(tmp)-1); 
	SVRUTILS::StrRpl(tmp, status_desc, 1024, "\\", "\\\\");

	memset(tmp, 0, 1024); 
	strncpy(tmp, status_desc,1023);  
	SVRUTILS::StrRpl(tmp, status_desc, 1024, "\r", "\\r");

	memset(tmp, 0, 1024);  
	strncpy(tmp, status_desc,1023);  
	SVRUTILS::StrRpl(tmp, status_desc, 1024, "\n", "\\n");

    CRCJson tmpJson;

    tmpJson.Add("isplug",               SvrPrinter::Inst().lp()->is_pluging);
    tmpJson.Add("ishavemedia",          SvrPrinter::Inst().lp()->is_have_media);
    tmpJson.Add("cd_src",               SvrPrinter::Inst().lp()->media_address);
    tmpJson.Add("tri_color_ink_level",  SvrPrinter::Inst().lp()->tri_color_ink_level);
    tmpJson.Add("black_color_ink_level",SvrPrinter::Inst().lp()->black_color_ink_level);
    tmpJson.Add("status",               SvrPrinter::Inst().lp()->status);
    tmpJson.Add("error_state",          SvrPrinter::Inst().lp()->error_state);
    tmpJson.Add("status_code",          SvrPrinter::Inst().lp()->status_code);
    tmpJson.Add("status_desc","");

    jdata.Add("Printer", tmpJson);

    CRCJson ret;
    ret.Add("data", jdata.ToString());
    m_csCtrl.response(*pJson, ret);    

    return 0;
}

int  
MachineServer::cdrom_return_disc_manual(CRCJson * pJson)
{
    CRCLog_Info("WAITTING return disc FINISH...");

	for (int i =0; i < RECORDERMAX; i++)
	{
		if (m_storage.record_attr[i].is_pluging && m_storage.record_attr[i].is_have_media == PRESENCE)
		{
			int ret = cdrom_return_disc(MIDAS_ELMADR_DT+i);
            if (ret != 0){break;}
		}		
	}   
    
    CRCLog_Info("RETURN disc FINISH");

    m_current_state = RUN;

    CRCJson ret;
    ret.Add("data", "reutrn disc finish");
    m_csCtrl.response(*pJson, ret);   

    //释放
    delete pJson;

    return 0;
}

int  
MachineServer::mailbox_export_disc_manual(CRCJson * pJson)
{
    std::string from        = "mailbox_export_disc_manual";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;

    //组合命令
    char cmd_buff[128]      = {0};
    CRCJson jParam;
    int cdaddr              = -1;
    if (pJson->Get("param", jParam) && jParam.Get("cdaddr",cdaddr) && cdaddr>0){
        sprintf(cmd_buff,"EXPO,0x%04x,\n",cdaddr);
    } else {
        CRCJson ret;
        ret.Add("data", "lost param cdaddr or cdaddr illegal");
        m_csCtrl.response(*pJson, ret);    
        delete pJson;
        return -1;
    }

    CRCLog::Info("mailbox_export_disc_manual cmd: %s",cmd_buff);
    
    //生成任务对象
    pJson->Add("cmdTransfer",    cmd_buff);
    pJson->Add("from",           from);
    pJson->Add("fromId",         fromId);
    pJson->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pJson);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pJson);
    }

    CRCLog_Info("mailbox_export_disc_manual WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "mailbox_export_disc_manual recv timeout", rm))
    {
        return -1;
    }

    if (strncmp(rm->data,"RET,0",5) == 0)
    //如果成功则修改m_storage的数据
    {
        sprintf(cmd_buff,"rm -Rf %s/%04d_*",SVRUTILS::MIRRORPATH,cdaddr);
        SVRUTILS::SystemExec(cmd_buff);
        //generate grep info 1:have magserial ,move to offline ,else delete it
        if (strlen(const_cast<char*>(m_storage.mag_slotarray[(cdaddr-1)/MAGITEMMAX].magazine.serial)) >0)
        {
            snprintf(
                cmd_buff,sizeof(cmd_buff),"mv  -f %s/%04d-%s.grep  %s/%04d-%s.offline",
                SVRUTILS::DINFOPATH,
                cdaddr,
                m_storage.mag_slotarray[(cdaddr-1)/MAGITEMMAX].magazine.serial,
                SVRUTILS::DINFOPATH,
                cdaddr%MAGITEMMAX,
                m_storage.mag_slotarray[(cdaddr-1)/MAGITEMMAX].magazine.serial);
        }else{
            snprintf(cmd_buff,sizeof(cmd_buff),"rm -Rf %s/%04d*",SVRUTILS::DINFOPATH,cdaddr);
        }
        SVRUTILS::SystemExec(cmd_buff);
        //delete info
        snprintf(cmd_buff,sizeof(cmd_buff),"rm -Rf %s/%04d.info",SVRUTILS::DINFOPATH,cdaddr);
        SVRUTILS::SystemExec(cmd_buff);
        //clear share mem status
        memset((void*)&(m_storage.media_attr[cdaddr-1]),0,sizeof(MediaAttr));
        //set local map remember mailbox cd_src
        m_storage.midas_box.mailbox_slotarray[0].maibox.cd_src_addr =  cdaddr;
        //set local madia attr
        m_storage.media_attr[cdaddr-1].trayexist    = NOPRESENCE;
        m_storage.media_attr[cdaddr-1].cdexist      = NOPRESENCE;
    } else {
        CRCLog::Error("MAILBOX EXPORT DISC FAILED: %s", rm->data);
    }

    CRCJson ret;
    ret.Add("data", rm->data);
    m_csCtrl.response(rm->json, ret);    

    delete rm;

    return 0;
}

int  
MachineServer::mailbox_import_disc_manual(CRCJson * pJson)
{
    std::string from        = "mailbox_import_disc_manual";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;
    CRCJson     ret;

    //组合命令
    char cmd_buff[128]      = {0};
    int  cdaddr             = -1;
    sprintf(cmd_buff,"HIDE,0x%04x,\n",MIDAS_ELMADR_IE);

    CRCLog::Info("mailbox_import_disc_manual cmd: %s",cmd_buff);
    
    //生成任务对象
    pJson->Add("cmdTransfer",    cmd_buff);
    pJson->Add("from",           from);
    pJson->Add("fromId",         fromId);
    pJson->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pJson);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pJson);
    }

    CRCLog_Info("mailbox_import_disc_manual WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "mailbox_import_disc_manual recv timeout", rm))
    {
        ret.Add("data", "mailbox_import_disc_manual recv timeout");
        m_csCtrl.response(rm->json, ret);    
        delete pJson;
        return -999;
    }

    if (strncmp(rm->data,"RET,0,me",8) == 0)
    {
        cdaddr = m_storage.midas_box.mailbox_slotarray[0].maibox.cd_src_addr;
        m_storage.midas_box.mailbox_slotarray[0].maibox.cd_src_addr = MIDAS_ELMADR_IE;
        //set media isexist status after mailimport
        m_storage.media_attr[cdaddr-1].trayexist    = PRESENCE;
        m_storage.media_attr[cdaddr-1].cdexist      = PRESENCE;
        m_storage.media_attr[cdaddr-1].ischecked    = 0;
        
        ret.Add("data", "success");
    }
    else if (strncmp(rm->data,"RET,0,1,",8) == 0)
    {
        cdaddr =  m_storage.midas_box.mailbox_slotarray[0].maibox.cd_src_addr;
        m_storage.midas_box.mailbox_slotarray[0].maibox.cd_src_addr = MIDAS_ELMADR_IE;
        //set media isexist status after mailimport
        m_storage.media_attr[cdaddr-1].trayexist    = 1;
        m_storage.media_attr[cdaddr-1].cdexist      = NOPRESENCE;
        m_storage.media_attr[cdaddr-1].ischecked    = 1;
        m_storage.media_attr[cdaddr-1].slot_status  = 'N';  //标记为未盘点状态

        ret.Add("data", "success");
    }
    else
    {
        CRCLog::Error("MAILBOX IMPORT DISC FAILED: %s", rm->data);
        ret.Add("data", rm->data);
    }

    m_csCtrl.response(rm->json, ret);    
    delete rm;

    return 0;
}

int  
MachineServer::printer_export_disc_manual(CRCJson * pJson)
{
    std::string from        = "printer_export_disc_manual";
    uint32_t    fromId      = m_atomic++;
    RetMessage* rm          = nullptr;
    CRCJson     ret;

    //组合命令
    char cmd_buff[128]      = {0};
    sprintf(cmd_buff,"PROU,0x%04x,\n",MIDAS_ELMADR_PR);

    CRCLog::Info("printer_export_disc_manual cmd: %s",cmd_buff);

    //生成任务对象
    pJson->Add("cmdTransfer",    cmd_buff);
    pJson->Add("from",           from);
    pJson->Add("fromId",         fromId);
    pJson->Add("status",         "ready");

    //这个key值用来在结果集中查找返回结果
    std::string&& key = gen_key(*pJson);

    //将任务放入发送任务队列
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_task_queue.push(pJson);
    }

    CRCLog_Info("printer_export_disc_manual WAITTING msg key<%s> return ...", key.c_str());

    if (!wait_for(key, "printer_export_disc_manual recv timeout", rm))
    {
        ret.Add("data", "printer_export_disc_manual recv timeout");
        m_csCtrl.response(*pJson, ret);    
        delete pJson;
        return -999;
    }

    if (strncmp(rm->data,"RET,0",5) != 0)
    {
        ret.Add("data", "success");
    }else{
        CRCLog_Error("printer_export_disc_manual failed! msg: %s\n", rm->data);
        ret.Add("data", rm->data);
    }

    m_csCtrl.response(rm->json, ret);    

    delete rm;

    return 0;
}

int  
MachineServer::printer_import_disc_manual(CRCJson * pJson)
{
    std::string from            = "printer_import_disc_manual";
    uint32_t    fromId          = m_atomic++;
    RetMessage* rm              = nullptr;
    CRCJson     ret;

    //组合命令
    char        cmd_buff[128]   = {0};
    CRCJson     jParam;
    int         cdaddr          = -1;
    std::string errStr;
    if (pJson->Get("param", jParam) && jParam.Get("cdaddr",cdaddr) && cdaddr>0){
        if (move_disc2eqpt(MIDAS_ELMADR_PR, cdaddr, &errStr) == 0){
            ret.Add("data", "success");
        } else {
            ret.Add("data", errStr);
        }
    } else {
        ret.Add("data", "lost param cdaddr or cdaddr illegal");
    }

    m_csCtrl.response(*pJson, ret);    
    delete pJson;
    return 0;
}