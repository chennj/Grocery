#include "svr_machine.h"

int main(int argc, char* args[])
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("LoginServerLog", "w", false);
	CRCConfig::Instance().Init(argc, args);

	MachineServer server;
	server.Init();
	while (true)
	{
		server.Run();
	}
	server.Close();

	CRCLog_Info("exit.");

	return 0;
}