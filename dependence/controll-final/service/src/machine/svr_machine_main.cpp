#include "svr_machine.h"
#include "svr_utils.h"

int main(int argc, char* args[])
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("MachineServerLog", "w", false);
	CRCConfig::Instance().Init(argc, args);

	//建立设备子服务运行目录
	char err[256] = {0};
	if (!SVRUTILS::GetMainPath(err,256)){
		CRCLog_Error("start server failed, %s", err);
		return 0;
	}
	
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