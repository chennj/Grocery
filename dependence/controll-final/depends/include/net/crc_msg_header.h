/**
 * 
 * author:  chenningjiang
 * desc:    结构体传输协议
 * 
 * */
#ifndef _CRC_MSG_HEADER_H_
#define _CRC_MSG_HEADER_H_

#include <vector>
#include <string>

enum CRCCMD
{
	CMD_LOGIN = 10,
	CMD_LOGIN_RESULT,
	CMD_AUTH,
	CMD_AUTH_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_C2S_HEART,
	CMD_S2C_HEART,
	CMD_EQPT_MOVE,								//移动光盘-》邮箱/打印机/光驱
	CMD_EQPT_MOVE_RESULT,
	CMD_EQPT_CHECK_SLOT,						//检查盘
	CMD_EQPT_CHECK_SLOT_RESULT,
	CMD_EQPT_CDROM_STATUS,						//获取光驱状态
	CMD_EQPT_CDROM_STATUS_RESULT,
	CMD_EQPT_CDROM_QUERY,						//获取光驱信息
	CMD_EQPT_CDROM_QUERY_RESULT,
	CMD_EQPT_CDROM_IN,							//光驱进
	CMD_EQPT_CDROM_IN_RESULT,
	CMD_EQPT_CDROM_OUT,							//光驱出
	CMD_EQPT_CDROM_OUT_RESULT,
	CMD_EQPT_IDLE_CDROM,						//获取空闲光驱
	CMD_EQPT_IDLE_CDROM_RESULT,
	CMD_EQPT_MAILBOX_QUERY,						//获取邮箱信息
	CMD_EQPT_MAILBOX_QUERY_RESULT,
	CMD_EQPT_MAILBOX_IN,						//邮箱进
	CMD_EQPT_MAILBOX_IN_RESULT,
	CMD_EQPT_MAILBOX_OUT,						//邮箱出
	CMD_EQPT_MAILBOX_OUT_RESULT,				
	CMD_EQPT_PRINTER_QUERY,						//获取打印机信息
	CMD_EQPT_PRINTER_QUERY_RESULT,				
	CMD_EQPT_PRINTER_IN,						//打印机进
	CMD_EQPT_PRINTER_IN_RESULT,					
	CMD_EQPT_PRINTER_OUT,						//打印机出
	CMD_EQPT_PRINTER_OUT_RESULT,
	CMD_EQPT_RETURNDISC,						//回盘
	CMD_EQPT_RETURNDISC_RESULT,
	CMD_EQPT_DISC_QUERY,						//获取光盘信息
	CMD_EQPT_DISC_QUERY_RESULT,
	CMD_EQPT_DISK_UPDATE,						//更新光盘信息
	CMD_EQPT_DISK_UPDATE_RESULT,		
	CMD_EQPT_OPENDOOR,							//打开盘库的门
	CMD_EQPT_OPENDOOR_RESULT,
	CMD_EQPT_EXPORT,							//出盘
	CMD_EQPT_EXPORT_RESULT,
	CMD_EQPT_IMPORT,							//入盘
	CMD_EQPT_IMPORT_RESULT,						
	CMD_EQPT_SYNC_DATA,							//同步盘库数据
	CMD_EQPT_SYNC_DATA_RESULT,
	CMD_STATION_QUERY,							//获取站点信息
	CMD_STATION_QUERY_RESULT,
	CMD_STATION_DISKINFO_QUERY,					//获取磁盘信息
	CMD_STATION_DISKINFO_QUERY_RESULT,				
	CMD_STATION_PRINT,							//打印光盘
	CMD_STATION_PRINT_RESULT,
	CMD_STATION_BURN,							//刻录光盘
	CMD_STATION_BURN_RESULT,
	CMD_STATION_PERFORMANCE_QUERY,				//站点性能查询
	CMD_STATION_PERFORMANCE_QUERY_RESULT,
	CMD_STATION_RESOURCE_QUERY,					//站点资源查询
	CMD_STATION_RESOURCE_QUERY_RESULT,
	CMD_STATION_TASK_QUERY,						//站点任务查询
	CMD_STATION_TASK_QUERY_RESULT,			
	CMD_ERROR
};

//消息头：所有结构体消息的基类
struct CRCDataHeader
{
	CRCDataHeader()
	{
		dataLength = sizeof(CRCDataHeader);
		cmd = CMD_ERROR;
	}
	unsigned short dataLength;
	unsigned short cmd;
	int sessionId;
	int msgId;
};

struct CRCLogin : public CRCDataHeader
{
	CRCLogin()
	{
		dataLength = sizeof(CRCLogin);
		cmd = CMD_LOGIN;
	}
	char userName[32];
	char PassWord[32];
	char data[28];
};

struct CRCLoginR : public CRCDataHeader
{
	CRCLoginR()
	{
		dataLength = sizeof(CRCLoginR);
		cmd = CMD_LOGIN_RESULT;
	}
	int result;
	char data[88];
};

struct CRCLogout : public CRCDataHeader
{
	CRCLogout()
	{
		dataLength = sizeof(CRCLogout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct CRCLogoutR : public CRCDataHeader
{
	CRCLogoutR()
	{
		dataLength = sizeof(CRCLogoutR);
		cmd = CMD_LOGOUT_RESULT;
	}
	int result;
};

struct CRCNewUserJoin : public CRCDataHeader
{
	CRCNewUserJoin()
	{
		dataLength = sizeof(CRCNewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
	}
	int scok;
};

struct CRCc2s_Heart : public CRCDataHeader
{
	CRCc2s_Heart()
	{
		dataLength = sizeof(CRCc2s_Heart);
		cmd = CMD_C2S_HEART;
	}
};

struct CRCs2c_Heart : public CRCDataHeader
{
	CRCs2c_Heart()
	{
		dataLength = sizeof(CRCs2c_Heart);
		cmd = CMD_S2C_HEART;
	}
};

struct CRCEqptMove : public CRCDataHeader
{
	CRCEqptMove()
	{
		dataLength = sizeof(CRCEqptMove);
		cmd = CMD_EQPT_MOVE;
	}

	int discAddr;
	int srcAddr;
	int destAddr;
	char test[0];
};

struct CRCEqptMoveR : public CRCDataHeader
{
	CRCEqptMoveR()
	{
		dataLength = sizeof(CRCEqptMoveR);
		cmd = CMD_EQPT_MOVE_RESULT;
	}
	int discAddr;
	int srcAddr;
	int destAddr;
	char code[9] = {0};
};

struct CRCEqptCheckSlot : public CRCDataHeader
{
	CRCEqptCheckSlot()
	{
		dataLength = sizeof(CRCEqptCheckSlot);
		cmd = CMD_EQPT_CHECK_SLOT;
	}
	int discAddr;
	char code[9] = {0};
};

struct CRCEqptCheckSlotR : public CRCDataHeader
{
	CRCEqptCheckSlotR()
	{
		dataLength = sizeof(CRCEqptCheckSlotR);
		cmd = CMD_EQPT_CHECK_SLOT_RESULT;
	}
	int discAddr;
	char code[9] = {0};
};

struct CRCEqptCdromStatus : public CRCDataHeader
{
	CRCEqptCdromStatus()
	{
		dataLength = sizeof(CRCEqptCdromStatus);
		cmd = CMD_EQPT_CDROM_STATUS;
	}
	int cdromAddr;
};

struct CRCEqptCdromStatusR : public CRCDataHeader
{
	CRCEqptCdromStatusR()
	{
		dataLength = sizeof(CRCEqptCdromStatusR);
		cmd = CMD_EQPT_CDROM_STATUS_RESULT;
	}
	int cdromAddr;
	char code[9] = {0};
};

struct CRCEqptCdromQuery : public CRCDataHeader
{
	CRCEqptCdromQuery()
	{
		dataLength = sizeof(CRCEqptCdromQuery);
		cmd = CMD_EQPT_CDROM_QUERY;
	}
};

struct CRCEqptCdromQueryR : public CRCDataHeader
{
	CRCEqptCdromQueryR()
	{
		dataLength = sizeof(CRCEqptCdromQueryR);
		cmd = CMD_EQPT_CDROM_QUERY_RESULT;
	}
	char data[1024];
	char code[9] = {0};
};

struct CRCEqptCdromIn : public CRCDataHeader
{
	CRCEqptCdromIn()
	{
		dataLength = sizeof(CRCEqptCdromIn);
		cmd = CMD_EQPT_CDROM_IN;
	}
	int cdromAddr;
};

struct CRCEqptCdromInR : public CRCDataHeader
{
	CRCEqptCdromInR()
	{
		dataLength = sizeof(CRCEqptCdromInR);
		cmd = CMD_EQPT_CDROM_IN_RESULT;
	}
	int cdromAddr;
	char code[9] = {0};
};

struct CRCEqptCdromOut : public CRCDataHeader
{
	CRCEqptCdromOut()
	{
		dataLength = sizeof(CRCEqptCdromOut);
		cmd = CMD_EQPT_CDROM_OUT;
	}
	int cdromAddr;
};

struct CRCEqptCdromOutR : public CRCDataHeader
{
	CRCEqptCdromOutR()
	{
		dataLength = sizeof(CRCEqptCdromOutR);
		cmd = CMD_EQPT_CDROM_OUT_RESULT;
	}
	int cdromAddr;
	char code[9] = {0};
};
#endif //!_CRC_MSG_HEADER_H_