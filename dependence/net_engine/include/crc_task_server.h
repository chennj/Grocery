#ifndef _CRC_TASK_SERVER_H_
#define _CRC_TASK_SERVER_H_
// file: crc_task_server.h

#include<thread>
#include<mutex>
#include<list>
#include<functional>

#include "crc_thread.h"

//执行任务的服务类
class CRCTaskServer
{
public:
	//所属serverid
	int serverId = -1;

private:
	typedef std::function<void()> CRCTask;

private:
	//任务数据
	std::list<CRCTask> _tasks;
	//任务数据缓冲区
	std::list<CRCTask> _tasksBuf;
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//
	CRCThread _thread;

public:
	//添加任务
	void addTask(CRCTask task);

    //启动工作线程
	void Start();

	void Close();

protected:
	//工作函数
	void OnRun(CRCThread* pThread);
};

#endif