/**
 * 
 * author:  chenningjiang
 * desc:    信号量简单封装，主要用于{@CRCThread}
 * 
 * */
#ifndef _CRC_SEMAPHORE_H_
#define _CRC_SEMAPHORE_H_

#include <chrono>
#include <thread>
#include <condition_variable>

//信号量
class CRCSemaphore
{
public:
	//阻塞当前线程
	void wait();

	//唤醒当前线程
	void wakeup();

private:
	//改变数据缓冲区时需要加锁
	std::mutex _mutex;
	//阻塞等待-条件变量
	std::condition_variable _cv;
	//等待计数
	int _wait = 0;
	//唤醒计数
	int _wakeup = 0;
};

#endif // !_CRC_SEMAPHORE_H_

//虚假唤醒