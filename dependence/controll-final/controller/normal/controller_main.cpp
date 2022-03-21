#include "crc_log.h"
#include "crc_config.h"
#include "controller.h"

int main(int argc, char* args[])
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("NlController", "w", false);
	CRCConfig::Instance().Init(argc, args);

	NlController server;
	server.Init();
	while (true)
	{
		server.Run();
	}
	server.Close();

	////在主线程中等待用户输入命令
	//while (true)
	//{
	//	char cmdBuf[256] = {};
	//	scanf("%s", cmdBuf);
	//	if (0 == strcmp(cmdBuf, "exit"))
	//	{
	//		server.Close();
	//		break;
	//	}
	//	else {
	//		CELLLog_Info("undefine cmd");
	//	}
	//}

	CRCLog_Info("exit.");

	return 0;
}