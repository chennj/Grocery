/**
 * 
 * author:  chenningjiang
 * desc:    设备子服务处理器
 * 
 * */
#ifndef _SVR_MACHINE_H_
#define _SVR_MACHINE_H_

#define _DEBUG

#include "crc_net_client_c.h"
#include "crc_easy_txt_client.h"
#include "crc_thread.h"
#include "svr_machine_midasbox.h"
#include "svr_machine_recorder.h"
#include "svr_machine_printer.h"
#include "crc_thread_pool.h"
#include "dvd+rw-mediainfo.h"

#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <atomic>

#define CMD_ACTION                  "cs_machine_action"

//设备（at91）推送过来的异常消息
#define DOOROPENREQUEST	            "DOOROPENREQUEST"
#define MAGPLUGOUT			        "MAGPLUGOUT"
#define DOOROPEN			        "DOOROPEN"
#define MAGPLUGIN			        "MAGPLUGIN"
#define DOORCLOSE			        "DOORCLOSE"

//命令
#define GMAP                        "GMAP,\n"

#define MAILBOXIN                   "mailbox in"
#define MAILBOXOUT                  "mailbox out"
#define INVENTORYTEST               "inventory"
#define RETURN_DISC                 "return disc"
#define QUERY_STATION               "query station"
#define MAILBOX_EXPORT_DISC         "mailbox export disc"
#define MAILBOX_IMPORT_DISC         "mailbox import disc"
#define PRINTER_EXPORT_DISC         "printer export disc"
#define PRINTER_IMPORT_DISC         "printer import disc"
#define MOVE_DISC_TO_CDROM          "move disc to cdrom"

#define MAILBOXIN_TRANSFER          "MLIN,0x6001,\n"
#define MAILBOXOUT_TRANSFER         "MLOU,0x6001,\n"

class MachineServer
{
public:
    //运行状态
    enum MachineState : int32_t{
        EXCEPTION =-1,
        INIT = 0,           //初始
        RUN,                //正常
        INVENTORY,          //盘点
        RETURNDISC          //回盘
    };

    //当前状态
    volatile MachineState   m_current_state = INIT;

    //任务类型
    enum MachineTaskType{
        UPDATEMIDASBOX=1,
        AUTOINVENTORY,
        NORMAL
    };

    //光盘信息，有两部分组成，一部分从at91传来，一部分由总控通过光驱识别产生
    typedef struct _MediaAttr{
        unsigned short      media_addr;
        unsigned short      slot_status;
        char                cdexist;
        char                trayexist;
        char                ischecked;
        char                isblank:4;
        char                isappendable:4;
        SDiskTypeInfo       diskTypeInfo;  
    }                       MediaAttr;

    typedef volatile struct _Storage{
        int                 try_burn_times;	                    //尝试烧录的最多次数，默认10次
        int			        burn_speed_bd;		                //default 4
        int			        burn_speed_dvd;		                //default 8
        char		        burn_stream_recording_switch[128];  //default ""
        //统计设备中已经用的容量和空盘容量
        long long	        storage_available;
        long long	        storage_used;
        //scsi host
        char		        scsi_host[12];
        //设备属性，从at91获得
        MidasBox            midas_box;
        //光驱属性，从at91获得
        RecordAttr          record_attr[RECORDERMAX];
        //midasbox 属性,从at91得到
        MidasMagSlot        mag_slotarray[MAGSLOTMAX];
        //光盘属性，通过盘点获得，与从at91得到的属性一一对应
        MediaAttr           media_attr[MAXSTATIONMEDIA];
    }                       Storage;

    Storage        m_storage;

    //组装从客户端及at91异步返回的信息
    typedef struct _RetMessage
    {
        CRCJson             json;       //来自客户端的命令数据
        char*               data;       //从at91返回的数据
        int                 datalen;    //从at91返回的数据长度

        _RetMessage(){}
        _RetMessage(_RetMessage& other){
            json = other.json;
            datalen = other.datalen;
            data = new char[datalen];
            memcpy(data,other.data,datalen);            
        }
        _RetMessage(CRCJson & j, const char* d, int l){
            json = j;
            datalen = l;
            data = new char[datalen];
            memcpy(data,d,datalen);
        }
        ~_RetMessage(){
            CRCLog_Info("~RetMessage exec...");
            delete data;
        }
    }                       RetMessage;
protected:
    //连接总控服务区的客户端
    //负责服务的注册，空闲心跳，自动重连
    CRCNetClientC           m_csCtrl;
    //连接设备服务器的客户端
    //负责服务的注册，空闲心跳，自动重连
    CRCEasyTxtClient        m_csMachine;
    //连接设备服务器的线程
    CRCThread               m_thread;
    //任务缓冲队列
    std::queue<CRCJson*>    m_task_queue;
    //任务缓冲队列锁
    std::mutex              m_task_queue_mtx;
    //是否需要重新获取 MidasBox
    bool                    m_isNeedGetMidasBox = true;
    //是否自动盘点
    bool                    m_isAutoInventory = true;
    //任务返回结果集
    std::map<std::string, RetMessage*> m_result_map;
    //结果集操作互斥量
    std::mutex              m_result_map_mtx;
    //线程池
    CRCThreadPool           m_thread_pool;
    //记录client
    CRCClientCTxt*          m_ptr_client_txt = nullptr;
    //消息id
    std::atomic<int>		m_atomic{ 0 };
    //条件
    std::condition_variable m_condition;
    //动作执行互斥量
    std::mutex              m_action_mtx;
    //设置属性互斥
    std::mutex              m_set_mtx;
public:
    virtual ~MachineServer();

public:
    void Init();

    void Run();

    void Close();

    void MachineLoop(CRCThread* pThread);

    inline CRCEasyTxtClient& machine(){return m_csMachine;}

    //处理从at91返回的消息
    virtual void OnProcess4Equipment(const char* pData, CRCClientCTxt* pTxtClient);

protected:
    //websocket
    void onopen_csCtrl(CRCNetClientC* client, CRCJson& msg);

    //api注册回调函数
    void cs_machine_action(CRCNetClientC* client, CRCJson& msg);

    //跟新 storage
    //信息来自 at91 的 MidasBox
    void update_storage_info();
    //盘点所有
    void inventory(CRCJson * pJson);
    //获取一个盘的信息
    //cd_addr：光盘地址
    //is_returndisc：是否将光盘送回
    void get_discinfo(uint32_t cd_addr, bool is_returndisc);
    //任务分发
    void do_action(MachineTaskType taskType, CRCJson * pJson = nullptr);
    //邮箱进
    void mailbox_in(CRCJson * pJson);
    //邮箱出
    void mailbox_out(CRCJson * pJson);
    //生成消息key
    std::string gen_key(const CRCJson & json);
    //wait for
    bool wait_for(const std::string& key, const std::string& msg, RetMessage*& pRmOut, uint32_t timeout = 60 * 1000);
    //移动光盘到光驱
    int  move_disc2cdrom(int cd_addr);
    //移动光盘到光驱
    int  move_auto_disc2cdrom(CRCJson * pJson);
    //获取空白光盘
    int  get_blank_disc(int cd_addr, char*mediatype, unsigned long long minSize, bool isSet=true);
    //检查光驱状态
    //返回 0：盘托已打开。-1：错误。
    int  check_cdrom_status(const char *cdrom_dev);
    //翻译光驱状态
    void trans_cdrom_status(int status, char* status_name);
    //移动光盘到设备：光驱、邮箱、打印机
    int  move_disc2eqpt(int dest_addr, int src_addr, std::string * errStr = nullptr);
    //关闭光驱
    int  cdrom_in(int cdrom_addr);
    //检测光驱与光盘
    int  check_cd_cdrom(int cd_addr, int cdrom_addr, int md5_check);
    //转换驱动器地址到设备名
    int  trans_addr2devname(int addr,char *dev_name);
    //循环检测cdrom是否已经就绪
    int  cdrom_is_ready(const char* cdrom_dev);
    //挂载光驱
    int  xmount (char *devname, char* mntdir, char *fstype, int cdrom_addr);
    //卸载光驱
    int  xumount(char *devname,int flag,int cdrom_addr);
    //
    int  is_blank_BD_RE(SDiskTypeInfo* info);
    //将光驱中光盘放回盘仓
    //0:成功，-1，失败，-2：源中无盘
    int  cdrom_return_disc(int cdrom_addr);
    //将所有光驱中的盘放回盘仓
    //0:成功，-1，失败
    int  cdrom_return_disc();
    //站点信息
    int  query_station(CRCJson * pJson);
    //光驱手动回盘（手动）
    int  cdrom_return_disc_manual(CRCJson * pJson);
    //邮箱出盘（手动）
    int  mailbox_export_disc_manual(CRCJson * pJson);
    //邮箱入盘（手动）
    int  mailbox_import_disc_manual(CRCJson * pJson);
    //打印机回盘（手动）
    int  printer_export_disc_manual(CRCJson * pJson);
    //打印机入盘（手动）
    int  printer_import_disc_manual(CRCJson * pJson);
};

#endif