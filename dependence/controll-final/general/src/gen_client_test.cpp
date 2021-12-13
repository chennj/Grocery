#include "gen_client.h"
#include "crc_config.h"

#include <atomic>
#include <list>
#include <vector>
using namespace std;

//服务端IP地址
const char * strIP = "127.0.0.1";
//服务端端口
uint16_t nPort = 4567;
//发送线程数量
int nThread = 1;
//客户端数量
int nClient = 1;
/*
::::::数据会先写入发送缓冲区
::::::等待socket可写时才实际发送
::每个客户端在nSendSleep(毫秒)时间内
::最大可写入nMsg条Login消息
::每条消息100字节（Login）
*/
//客户端每次发几条消息
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

std::atomic_int sendCount(0);
std::atomic_int readyCount(0);
std::atomic_int nConnect(0);

void WorkThread(CRCThread* pThread,int id)
{
	//n个线程 id值为 1~n
	CRCLog_Info("thread<%d>,start", id);
	//客户端数组
	vector<GeneralClient*> clients(nClient);
	//计算本线程客户端在clients中对应的index
	int begin = 0;
	int end = nClient/nThread;
	if (end < 1)
		end = 1;
	int nTemp1 = nSendSleep > 0 ? nSendSleep : 1;
	for (int n = begin; n < end; n++)
	{
		if (!pThread->isRun())
			break;
		clients[n] = new GeneralClient();
		clients[n]->_tRestTime = n % nTemp1;
		//多线程时让下CPU
		CRCThread::Sleep(0);
	}
	for (int n = begin; n < end; n++)
	{
		if (!pThread->isRun())
			break;
		if (INVALID_SOCKET == clients[n]->InitSocket(nSendBuffSize, nRecvBuffSize))
			break;
		if (SOCKET_ERROR == clients[n]->Connect(strIP, nPort))
			break;
		nConnect++;
		CRCThread::Sleep(0);
	}
	//所有连接完成
	CRCLog_Info("thread<%d>,Connect<begin=%d, end=%d ,nConnect=%d>", id, begin, end, (int)nConnect);

	readyCount++;
	while (readyCount < nThread && pThread->isRun())
	{//等待其它线程准备好，再发送数据
		CRCThread::Sleep(10);
	}

	//消息
	CRCEqptMove mmove;
	//给点有意义的值
	mmove.srcAddr  = 1;
	mmove.destAddr = 2;
	mmove.discAddr = 3;
	char cs[] = "this is a test!";
	memcpy(mmove.test, cs, sizeof(cs));
	mmove.dataLength += sizeof(cs);
	//
	//收发数据都是通过onRun线程
	//SendData只是将数据写入发送缓冲区
	//等待select检测可写时才会发送数据
	//旧的时间点
	auto t2 = CRCTime::getNowInMilliSec();
	//新的时间点
	auto t0 = t2;
	//经过的时间
	auto dt = t0;
	CRCTimestamp tTime;
	while (pThread->isRun())
	{
		t0 = CRCTime::getNowInMilliSec();
		dt = t0-t2;
		t2 = t0;
		//本次while (pThread->isRun())循环主要工作内容
		//代号work
		{
			int count = 0;
			//每轮每个客户端发送nMsg条数据
			for (int m = 0; m < nMsg; m++)
			{
				//每个客户端1条1条的写入消息
				for (int n = begin; n < end; n++)
				{
					if (clients[n]->isRun())
					{
						if (clients[n]->SendTest(&mmove,bChekSendBack) > 0)
						{
							++sendCount;
						}
					}
				}
			}
			//sendCount += count;
			for (int n = begin; n < end; n++)
			{
				if (clients[n]->isRun())
				{	//超时设置为0表示select检测状态后立即返回
					if (!clients[n]->OnRun(0))
					{	//连接断开
						nConnect--;
						continue;
					}
					//检测发送计数是否需要重置
					clients[n]->checkSend(dt);
				}
			}
		}
		CRCThread::Sleep(nWorkSleep);
	}
	//--------------------------
	//关闭消息收发线程
	//tRun.Close();
	//关闭客户端
	for (int n = begin; n < end; n++)
	{
		clients[n]->Close();
		delete clients[n];
	}
	CRCLog_Info("thread<%d>,exit", id);
	--readyCount;
}

int main(int argc, char *args[])
{
	//设置运行日志名称
	CRCLog::Instance().setLogPath("clientLog", "w", false);

	CRCConfig::Instance().Init(argc, args);

	strIP = CRCConfig::Instance().getStr("strIP", "127.0.0.1");
	nPort = CRCConfig::Instance().getInt("nPort", 4567);
	nThread = CRCConfig::Instance().getInt("nThread", 1);
	nClient = CRCConfig::Instance().getInt("nClient", 10000);
	nMsg = CRCConfig::Instance().getInt("nMsg", 10);
	nSendSleep = CRCConfig::Instance().getInt("nSendSleep", 100);
	nWorkSleep = CRCConfig::Instance().getInt("nWorkSleep", 1);
	bChekSendBack = CRCConfig::Instance().hasKey("-chekSendBack");
	nSendBuffSize = CRCConfig::Instance().getInt("nSendBuffSize", SEND_BUFF_SZIE);
	nRecvBuffSize = CRCConfig::Instance().getInt("nRecvBuffSize", RECV_BUFF_SZIE);

	//启动终端命令线程
	//用于接收运行时用户输入的指令
	CRCThread tCmd;
	tCmd.Start(nullptr, [](CRCThread* pThread) {
		while (true)
		{
			char cmdBuf[256] = {};
			scanf("%s", cmdBuf);
			if (0 == strcmp(cmdBuf, "exit"))
			{
				//pThread->Exit();
				CRCLog_Info("退出cmdThread线程");
				break;
			}
			else {
				CRCLog_Info("不支持的命令。");
			}
		}
	});

	//启动模拟客户端线程
	vector<CRCThread*> threads;
	for (int n = 0; n < nThread; n++)
	{
		CRCThread* t = new CRCThread();
		t->Start(nullptr, [n](CRCThread* pThread) {
			WorkThread(pThread, n+1);
		});
		threads.push_back(t);
	}
	//每秒数据统计
	CRCTimestamp tTime;
	while (tCmd.isRun())
	{
		auto t = tTime.getElapsedSecond();
		if (t >= 5.0)
		{
			CRCLog_Info("thread<%d>,clients<%d>,connect<%d>,time<%lf>,send<%d>",nThread, nClient, (int)nConnect,t, (int)sendCount);
			sendCount = 0;
			tTime.update();
		}
		CRCThread::Sleep(1);
	}
	//
	for (auto t : threads)
	{
		t->Close();
		delete t;
	}
	CRCLog_Info("。。已退出。。");
	return 0;
}

