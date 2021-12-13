#include "virtual_server.h"

int main(int argc,char* args[])
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("serverLog", "w", false);
	CRCConfig::Instance().Init(argc, args);

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

	VirtualServer server;
	server.InitSocket();
	server.Bind(strIP, nPort);
	server.Listen(SOMAXCONN);
	server.Start(nThread);

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
