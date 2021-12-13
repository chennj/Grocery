#include "svr_machine_client.h"
#include "svr_machine_server.h"

int nMsg = 1;
//写入消息到缓冲区的间隔时间
int nSendSleep = 1;
//工作休眠时间
int nWorkSleep = 1;
//客户端发送缓冲区大小
int nSendBuffSize = SEND_BUFF_SZIE;
//客户端接收缓冲区大小
int nRecvBuffSize = RECV_BUFF_SZIE;
//是否检测-发送的请求已被服务器回应
int bChekSendBack = true;

int main(int argc, char* argv[])
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("serverLog", "w", false);
	CRCConfig::Instance().Init(argc, argv);

	const char* strIP   = CRCConfig::Instance().getStr("strIP", "any");
	uint16_t nPort      = CRCConfig::Instance().getInt("nPort", 4567);
	int nThread         = CRCConfig::Instance().getInt("nThread", 1);

	if (CRCConfig::Instance().hasKey("-p"))
	{
		CRCLog_Info("hasKey -p");
	}

	if (strcmp(strIP, "any") == 0)
	{
		strIP = nullptr;
	}

    MachineClient client;
    if (INVALID_SOCKET == client.InitSocket(nSendBuffSize, nRecvBuffSize))
        return -1;
    if (SOCKET_ERROR == client.Connect("127.0.0.1", 7654))
        return -1;

	MachineServer server;
    server.AddMachineClient(&client);

	server.InitSocket();
	server.Bind(strIP, nPort);
	server.Listen(SOMAXCONN);
	server.Start(nThread);
    server.Send2CtrlServerLoop();

    client.Loop();
    client.Send2Stm32Loop();

	//在主线程中等待用户输入命令
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			server.Close();
			break;
		}
		else {
			CRCLog_Info("undefine cmd");
		}
	}

	CRCLog_Info("exit.");
	return 0;
}