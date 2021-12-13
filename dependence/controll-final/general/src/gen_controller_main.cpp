#include "crc_log.h"
#include "crc_config.h"
#include "gen_controller.h"

int main(int argc, char* args[])
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("GenControllerLog", "w", false);
	CRCConfig::Instance().Init(argc, args);

	GenController server;
	server.Init();
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