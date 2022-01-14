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

//设备（at91）推送过来的异常消息
#define DOOROPENREQUEST	    "DOOROPENREQUEST"
#define MAGPLUGOUT			"MAGPLUGOUT"
#define DOOROPEN			"DOOROPEN"
#define MAGPLUGIN			"MAGPLUGIN"
#define DOORCLOSE			"DOORCLOSE"

//命令
#define GMAP                "GMAP,\n"

class MachineServer
{
public:
    //state
    enum MachineState{
        EXCEPTION =-1,
        INIT = 0,           //初始
        RUN,                //正常
        INVENTORY           //盘点
    };
    //finite state machine 
    typedef struct _FSM{
        int StateId;
        MachineState State;
        std::string StateDesc;
    } FSM;
    //当前状态
    volatile MachineState   m_current_state = INIT;
    enum LocalTaskType{
        UPDATEMIDASBOX=1
    };

    /* 如果是BD空盘，大小根据media_capacity ,如果是DVD，默认是4G */
    typedef struct _DiskTypeInfo
    {
        int			        media_type;
        char 		        media_type_str[32];          /*检测出来的类型*/
        char                media_id[32];
        char 		        media_status_str[16];
        char		        media_number_session;
        char		        media_last_session_status[16];
        char		        media_number_tracks;
        
        unsigned int		media_block_size;
        char				media_format_status[16];
        unsigned long long	media_capacity;       
        unsigned long long	media_track_freesize;
        
        char		        media_writespeed_cnt;
        char		        media_writespeed_str[8][32];

        /* blkid get information */
        char		        label_name[128];
        /* for mount */
        char	            fs_type[16];
        /*  analyse data  */
        char                media_abilty;

    }                       DiskTypeInfo;

    /* 光盘信息，有两部分组成，一部分从at91传来，一部分由总控通过光驱识别产生 */
    typedef struct _MediaAttr{
        unsigned short      media_addr;
        unsigned short      slot_status;
        char                cdexist;
        char                trayexist;
        char                ischecked;
        char                isblank:4;
        char                isappendable:4;
        DiskTypeInfo        diskTypeInfo;  
    }                       MediaAttr;

    typedef struct _{
        int                 try_burn_times;	                    //尝试烧录的最多次数，默认10次
        int			        burn_speed_bd;		                //default 4
        int			        burn_speed_dvd;		                //default 8
        char		        burn_stream_recording_switch[128];  //default ""
        /* 统计设备中已经用的容量和空盘容量 */
        long long	        storage_available;
        long long	        storage_used;
        /* scsi host */
        char		        scsi_host[12];
        /* midasbox 属性,从at91得到  */
        MidasMagSlot        mag_slotarray[MAGSLOTMAX];
        /* 光盘属性，通过盘点获得，与从at91得到的属性一一对应 */
        MediaAttr           media_attr[MAXSTATIONMEDIA];
    }                       Storage;

    Storage                 m_storage;
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
    //总控任务缓冲队列锁
    std::mutex              _task_queue_mtx;
    //本地任务队列
    //优先级高于客户端发过来的任务队列即（_task_queue）
    //一般用于获取at91或以后的stm32的设备信息，在第一次连接、断线重连、异常发生
    //（如，开门等）
    std::queue<CRCJson*>    _local_task_queue;
    //本地任务缓冲队列锁
    std::mutex              _local_task_queue_mtx;
    //是否需要重新获取 MidasBox
    bool                    _isNeedGetMidasBox = true;
public:
    virtual ~MachineServer();

public:
    void Init();

    void Run();

    void Close();

    void AddLocalTask(LocalTaskType localTaskType);

    //分析客户端的命令是否合法
    virtual bool ParseCmd(const CRCJson & json, std::string & cmd) const;

    //处理从at91返回的消息
    virtual void OnProcess4Equipment(std::string& str4Eqpt);

    void MachineLoop(CRCThread* pThread);

    inline CRCEasyTxtClient& machine(){return _csMachine;}

private:
    void onopen_csCtrl(CRCNetClientC* client, CRCJson& msg);

    void cs_machine_mailbox(CRCNetClientC* client, CRCJson& msg);

    void cs_machine_query(CRCNetClientC* client, CRCJson& msg);

    //跟新 storage
    //信息来自 at91 的 MidasBox
    void update_storage_info(std::string& ss);
};

#endif