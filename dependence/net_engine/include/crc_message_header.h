#ifndef _CRC_MESSAGE_HEADER_H_
#define _CRC_MESSAGE_HEADER_H_
// file: crc_message_header.h

enum CMD
{
    CMD_LOGIN = 10,
	CMD_LOGIN_RESULT,
	CMD_LOGOUT,
	CMD_LOGOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_C2S_HEART,
	CMD_S2C_HEART,
	CMD_ERROR
};

struct CRCDataHeader
{
    unsigned short dataLength;
	unsigned short cmd;

    CRCDataHeader()
	{
		dataLength = sizeof(CRCDataHeader);
		cmd = CMD_ERROR;
	}
};

// 登录及返回登录结果
// -----------------------------------------------------------------------
struct Login : public CRCDataHeader
{
public:
	Login()
	{
		dataLength = sizeof(Login);
		cmd = CMD_LOGIN;
	}

	char username[32];
	char password[32];
	char data[100 - 68];
};

struct LoginR : public CRCDataHeader
{
	LoginR()
	{
		dataLength = sizeof(LoginR);
		cmd = CMD_LOGIN_RESULT;
		result = 0;
	}
	int result;
	char data[88];
	int msgID;
};
// -----------------------------------------------------------------------

// 注销及返回注销结果
struct Logout : public CRCDataHeader
{
	Logout()
	{
		dataLength = sizeof(Logout);
		cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct LogoutR : public CRCDataHeader
{
	LogoutR()
	{
		dataLength = sizeof(LogoutR);
		cmd = CMD_LOGOUT_RESULT;
		result = 0;
	}
	int result;
};
// -----------------------------------------------------------------------

// 新用户加入
// -----------------------------------------------------------------------
struct NewUserJoin : public CRCDataHeader
{
	NewUserJoin()
	{
		dataLength = sizeof(NewUserJoin);
		cmd = CMD_NEW_USER_JOIN;
		scok = 0;
	}
	int scok;
};
// -----------------------------------------------------------------------

// 心跳，客户端-服务端 / 服务端-客户端
// -----------------------------------------------------------------------
struct C2SHeart : public CRCDataHeader
{
	C2SHeart()
	{
		dataLength = sizeof(C2SHeart);
		cmd = CMD_C2S_HEART;
	}
};

struct S2CHeart : public CRCDataHeader
{
	S2CHeart()
	{
		dataLength = sizeof(S2CHeart);
		cmd = CMD_S2C_HEART;
	}
};
// -----------------------------------------------------------------------

#endif