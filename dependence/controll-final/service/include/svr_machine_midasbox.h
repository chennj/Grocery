/**
 * 
 * author:  chenningjiang
 * desc:    设备数据类
 * 
 * */
#ifndef _SVR_MACHINE_MIDASBOX_H_
#define _SVR_MACHINE_MIDASBOX_H_

#define MAGSLOTMAX              128                 //最大盘仓数，依赖于单片机上的gpio数目
#define MAILBOXMAX              1                   //最大邮箱数
#define RECORDERMAX             32                  //最大光驱数
#define BRIDGEMAX               1                   //未知，待定

#define MAXSTATIONMEDIA         MAGSLOTMAX*50       //最大盘槽数，每个盘仓50个槽，可放50张光盘

#define RFIDSTRMAX              16                  //盘仓电子标签长度
#define MAGITEMMAX              50                  //盘仓中盘槽数量

#define Q_PRT_STATUS			0x01
#define GO_PUT_DISC				0x02
#define GO_PRT_READY			0x03
#define CLEAN_HEAD				0x04
#define INITIALIZE				0x05
#define UPGRADE					0x06
#define SHUTDOWN				0x0A

#define  GENBERR	  			-1
#define  XNETDOWN	  			-3
#define  NOBLANKDISC  			-2
#define  XORISOERR    			-4
#define  NODISC		  			-5
#define  NOINFO		  			-6
#define  DUPDIRERR	  			-7					//盘点失败
#define  BAD_MEDIA    			-8


/* 存在标志 presence_flag 类型 */
#define  PRESENCE   			1					//存在
#define  NOPRESENCE 			2					//不存在
#define  UNKNOWN    			0					//未知

#define MIDAS_ELMADR_LIB		0x0000				//盘库自身
#define MIDAS_ELMADR_ST			0x0001				//第一张光盘
#define MIDAS_ELMADR_MT			0x4001				//盘库机械手
#define MIDAS_ELMADR_DT			0x5001				//第一个驱动器	
#define MIDAS_ELMADR_IE			0x6001				//邮箱首地址
#define MIDAS_ELMADR_PR			0x7001				//打印机
#define MIDAS_ELMADR_MAG		0x8001				//直接指定盘仓号，则对应了盘仓电子标签
#define MIDAS_ELMADR_ALL		0x9001

#define MIDAS_ELEM_DOOR 		0xFFFF  			//最下面,对应机械手开门，打包模式
#define MIDAS_ELEM_PANEL 		0xFFFE  			//对应机械手panel open模式

#define CDS_NO_INFO             0       			// if not implemented 
#define CDS_NO_DISC             1
#define CDS_TRAY_OPEN           2
#define CDS_DRIVE_NOT_READY     3
#define CDS_DISC_OK             4

/* LED3 POWER	LED2 PAPER	LED1 INK	LPC 2378
OFF	闪烁	闪烁	0x0C 字车错误
OFF	OFF	OFF	0x10 打印机关机
ON	ON	OFF	0x06 打印机缺纸
ON	闪烁	OFF	0x09 打印机夹纸
ON	OFF	闪烁	0x0A 墨水少
ON	OFF	ON	0x07 缺墨或墨盒出现故障
ON	闪烁	闪烁	0x0B打印机导板错误
ON	OFF	OFF	0x00打印机正常状态
闪烁	不变	不变	0x0D打印机自检
*/

#define COMBCMDOMAX             16

#define CMDLENMAX               64
#define CMDNUMS                 32

/////////////////////////////////////////////////////////////////////
//MidasAttr
/////////////////////////////////////////////////////////////////////
typedef struct _MidasAttr{
	/* box name */
	char                        box_name[32];
	/* max mag num */   
	unsigned short              mag_max;
	/* max  recorder num */ 
	unsigned short              rec_max;
	/* max mail */  
	unsigned short              mail_max;
}__attribute__((packed))        MidasAttr;

/////////////////////////////////////////////////////////////////////
//MidasNet
/////////////////////////////////////////////////////////////////////
typedef struct  _MidasCmd{
	/* 当前命令指针 */
	unsigned char               cmd_pi;
	/* cmd buff */  
	char                        cmd_buff[CMDNUMS][CMDLENMAX];
	/* 执行结果  */ 
	char                        cmd_y2no[CMDNUMS];
}__attribute__((packed))        MidasCmd;

/////////////////////////////////////////////////////////////////////
//MidasNet
/////////////////////////////////////////////////////////////////////
//IP 息
typedef struct stIPInfo
{
	unsigned short		        IP1;
	unsigned short		        IP2;
	unsigned short		        IP3;
	unsigned short		        IP4;
}__attribute__((packed))	    SIPInfo;

//Mac 信息
typedef struct stMacInfo         
{
	unsigned short 	            mac1;
	unsigned short 	            mac2;
	unsigned short 	            mac3;
	unsigned short 	            mac4;
	unsigned short 	            mac5;
	unsigned short 	            mac6;
}__attribute__((packed))        SMacInfo;

/* 网络服务描述 */	
typedef struct _MidasNet{
	/* mac address */
	SMacInfo                    smacinfo;
	/* ip */    
	SIPInfo                     sipinfo;
}__attribute__((packed))        MidasNet;

/////////////////////////////////////////////////////////////////////
//MidasDoor
/////////////////////////////////////////////////////////////////////
typedef struct _Door{
	char                        door_flag;	
}__attribute__((packed))        MidasDoor;

/////////////////////////////////////////////////////////////////////
//MidasBridgeSlot
/////////////////////////////////////////////////////////////////////
/* 盘托 */
typedef struct _MidasMagTray{
	/*　存在标志　*/
	unsigned char               tray_presence_flag;
}__attribute__((packed))        MidasMagTray ;

/* 蓝光盘 */
typedef struct _MidasBlueCd{
	/* 存在标志 */
	unsigned char               cd_presence_flag;
}__attribute__((packed))        MidasBlueCd;

/* 设备描述 */	
typedef struct _MidasBridge{
	
	/* 中有无盘托 */
	MidasMagTray                mag_tray;
	/* 中有无光盘 */    
	MidasBlueCd                 blue_cd;	
	/* 当有盘时，记录来源 ,启动时如有盘，按此源地址，搬运回去*/
	int                         cd_src_addr;
}__attribute__((packed))        MidasBridge ;

/* 插槽设备描述 */
typedef struct _MidasBridgeSlot{	
	/* 表时此slot上是否有桥 */
	char                        bridge_plug;
	/* 小车状态 */  
	char                        cab_pos;
	MidasBridge                 bridge;
}__attribute__((packed))        MidasBridgeSlot ;

/////////////////////////////////////////////////////////////////////
//MidasCarrier
/////////////////////////////////////////////////////////////////////
/*  add */
typedef struct _MidasTtysDevice{
	char                        ttys_dev_name[32];	
}__attribute__((packed))        MidasTtys;

typedef struct _ComboCmd{
	/* 命令源，传入打开的设备号 */
	int                         srcfd;
	char                        cmdtype;
	int                         arg;
}__attribute__((packed))        ComboCmd;

typedef struct _Carrier{
	/* 表明是否有机械手 */
	char carrier_plug;
	/* 如果有机械手，忙碌状态 */
	char                        carrier_busy;
	/* 与机械手相连的串口 */    
	MidasTtys                   tty_device;
	/* 盘槽 */  
	MidasMagTray                mag_tray;
	/* 光盘 */  
	MidasBlueCd                 blue_cd;
	/* 当机械手上有盘时，记录来源 */
	int                         cd_src_address;
	/* 复合命令标志 */
	char                        combo_flag;
	/* 复合命令长度 */  
	short                       combo_len;
	/* 当前命令位置 */  
	short                       combo_p;
	ComboCmd                    combo_cmd[COMBCMDOMAX];
}__attribute__((packed))        MidasCarrier;

/////////////////////////////////////////////////////////////////////
//MidasPrinter
/////////////////////////////////////////////////////////////////////
typedef struct prt_return
{
	unsigned char			    printer_status;
	unsigned char			    printer_hasdisc;
	unsigned char			    printer_pos;
	unsigned char			    printer_model[6];
	unsigned char			    printer_serial[6];
	unsigned char			    printer_errno;
	unsigned char			    is_printed;
	unsigned char			    reserved_b;
	unsigned char			    crc_high;
	unsigned char			    crc_low;
} SPrtReturn ;

typedef struct _MidasPrint{
	/* 表时此slot上是否有光驱 */
	char                        print_plug;
	/* 如果有光驱，忙碌状态 */
	char                        print_busy;
	/* 当打印机中有盘时，记录来源 ,启动时如果打印机有盘，按此源地址，搬运回去*/
	int                         cdrom_src_address;
	MidasMagTray                mag_tray;
	MidasBlueCd                 blue_cd;
	/* 打印机当前状态 */    
	SPrtReturn                  print_status;   
}__attribute__((packed))        MidasPrinter;	

/////////////////////////////////////////////////////////////////////
//MidasMagSlot
/////////////////////////////////////////////////////////////////////
/* 盘槽 */
typedef struct _MidasMagItem{
	/* 盘槽 */
	MidasMagTray                mag_tray;
	/* 光盘 */  
	MidasBlueCd                 blue_cd;
}__attribute__((packed))        MidasMagItem ;

/* 盘仓 */
typedef struct _MidasMagazine{
	/* 盘仓电子标签 */
	char                        serial[RFIDSTRMAX];
	/* 盘槽数目 */  
	MidasMagItem                mag_item[MAGITEMMAX];
}__attribute__((packed))        MidasMagazine ;

/* 盘仓插槽设备 */
typedef struct _MidasMagSlot{
	/* 表时此slot上是否有盘仓 */
	char                        mag_plug;
	/* 插槽后射频读取设备号 */
	char                        rfid_devminor;
	char                        rfid_telno;
	/*  插槽中盘仓 */   
	MidasMagazine               magazine;	
}__attribute__((packed))        MidasMagSlot;

/////////////////////////////////////////////////////////////////////
//MidasMailBoxSlot
/////////////////////////////////////////////////////////////////////
/* 邮箱设备描述 */	
typedef struct _MidasMailBox{
	
	/* 邮箱中有无盘托 */
	MidasMagTray                mag_tray;
	/* 邮箱中有无光盘 */    
	MidasBlueCd                 blue_cd;	
	/* 当邮箱中有盘时，记录来源 ,启动时如果邮箱有盘，按此源地址，搬运回去*/
	int                         cd_src_addr;
}__attribute__((packed))        MidasMailBox ;

/* 邮箱插槽设备描述 */
typedef struct _MailBoxrSlot{	
	/* 表时此slot上是否有邮箱 */
	char                        mailbox_plug;
	/* 如果有光驱，忙碌状态 */
	char                        mailbox_busy;
	MidasMailBox                maibox;
}__attribute__((packed))        MidasMailBoxSlot ;

/////////////////////////////////////////////////////////////////////
//MidasRecordeSlot
/////////////////////////////////////////////////////////////////////
/* 光驱设备描述 */	
typedef struct _MidasRecorder{
	MidasMagTray                mag_tray;
	MidasBlueCd                 blue_cd;		
	/* 当光驱中有盘时，记录来源 逻辑地址*/
	int                         cd_src_address;
} __attribute__((packed))       MidasRecorder;

typedef struct _MidasRecorderSlot{
	/* 表时此slot上是否有光驱 */
	char                        recorder_plug;
	/* 如果有光驱，忙碌状态 */
	char                        recorder_busy;
	/* 设备描述光盘和托的状态 */
	MidasRecorder               recorder;
} __attribute__((packed))       MidasRecordeSlot;

/////////////////////////////////////////////////////////////////////
//TotalCnt
/////////////////////////////////////////////////////////////////////
typedef struct _TotalCnt{
	unsigned int                prt_cnt;
	unsigned int                carr_cnt;
	unsigned int                mail_cnt;
	unsigned int                bridge_cnt;
	unsigned int                record_cnt[RECORDERMAX];
}__attribute__((packed))        TotalCnt;

/////////////////////////////////////////////////////////////////////
//MidasBox
/////////////////////////////////////////////////////////////////////
typedef struct _MidasBox{
	/* first start flag */
	char                        first_flag ;                
	/* not excute shutdown */
	char                        not_shutdown_flag ;
	/* box attribute */ 
	MidasAttr	                midas_attr;
	/* cmd */   
	MidasCmd                    midas_cmd;
	/* 网卡 */  
	MidasNet                    net;
	/* 机箱门 */    
	MidasDoor                   door;
	/* 机械手 */    
	MidasCarrier                carrier;
	/* 打印机 */    
	MidasPrinter                printer;
	/* 设备盘仓列，用于描述安装了多少盘仓 */
    MidasMagSlot                mag_slotarray[MAGSLOTMAX];
	/* 邮箱设备列 ，用于描述安装了多少个邮箱 */
	MidasMailBoxSlot            mailbox_slotarray[MAILBOXMAX];
	/* 光驱设备列 ，用于描述了多少光驱 */
	MidasRecordeSlot            recorde_slotArray[RECORDERMAX];
	/* 桥设备列 ，用于描述了多少桥 */
	MidasBridgeSlot             bridge_slotArray[BRIDGEMAX];
	/* 统计使用数量 */
	TotalCnt		            total_cnt;
}__attribute__((packed))        MidasBox;	

#endif //!_SVR_MACHINE_MIDASBOX_H_