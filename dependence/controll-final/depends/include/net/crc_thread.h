#ifndef _CRC_THREAD_H_
#define _CRC_THREAD_H_

#include "crc_semaphore.h"

#include <functional>

class CRCThread
{
public:
	static void Sleep(time_t dt);
private:
	typedef std::function<void(CRCThread*)> EventCall;
public:
	//启动线程
	void Start(
		EventCall onCreate = nullptr,
		EventCall onRun = nullptr,
		EventCall onDestory = nullptr);

	//关闭线程
	void Close();

	//在工作函数中退出
	//不需要使用信号量来阻塞等待
	//如果使用会阻塞
	void Exit();

	//线程是否启动运行状态
	bool isRun();

protected:
	//线程的运行时的工作函数
	void OnWork();
private:
	EventCall 		_onCreate;
	EventCall 		_onRun;
	EventCall 		_onDestory;
	//不同线程中改变数据时需要加锁
	std::mutex 		_mutex;
	//控制线程的终止、退出
	CRCSemaphore 	_sem;
	//线程是否启动运行中
	bool			_isRun = false;
};

#endif // !_CRC_THREAD_H_
