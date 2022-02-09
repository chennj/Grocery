/**
 * 
 * author:  chenningjiang
 * desc:    线程池
 * 
 * */
#ifndef _CRC_THREAD_POOL_H_
#define _CRC_THREAD_POOL_H_

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <chrono>
#include <utime.h>
#include <functional>

int64_t getNowMs();

#define TNOWMS	getNowMs()

class CRCThreadPool
{
protected:
	struct TaskFunc
	{
		TaskFunc(uint64_t expireTime):m_expireTime(expireTime)
		{}
		std::function<void()> m_func;
		int64_t m_expireTime = 0;
	};
	typedef std::shared_ptr<TaskFunc> TaskFuncPtr;

public:
	explicit CRCThreadPool();
	virtual ~CRCThreadPool();

public:
	//初始化
	//num：工作线程个数
	bool init(size_t num);

	//获取线程个数
	size_t getThreadNum();

	//获取当前线程池的任务数
	size_t getJobNum();

	//返回一个执行任务的 future，可以通过这个对象获取返回值
	//future<decltype(f(args...))> 返回future，调用者可以通过future获取返回值
	template<class F, class... Args>
	auto exec(int64_t timeoutMs, F&& f, Args&&... args)->std::future<decltype(f(args...))>
	{
		int64_t expireTime = (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);		//获取现在时间

		//返回返回值类型
		using RetType = decltype(f(args...));								//推导返回值

		//封装任务
		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);			//封装任务指针，设置过期时间
		fPtr->m_func = [task]() {
			//具体执行的函数
			(*task)();
		};

		std::unique_lock<std::mutex> lock(m_mutex);
		m_tasks.push(fPtr);			//插入任务
		m_condition.notify_one();	//唤醒阻塞的线程，可以考虑只有任务队列不为空的情况再去notify

		return task->get_future();
	}

	//返回一个执行任务的 future，可以通过这个对象获取返回值
	template<class F, class... Args>
	auto exec(F&& f, Args&&... args)->std::future<decltype(f(args...))>
	{
		return exec(0, f, args...);
	}

	//等待当前任务队列中，所有工作全部结束（队列无任务）
	//millsecond	-1:永远等待
	//返回			true, 所有工作都处理完毕
	//				false,超时退出
	bool waitForAllDone(int millsecond = -1);

	//启动线程池
	bool start();

	//停止线程池
	void stop();

protected:
	//获取任务队列中的任务执行函数
	bool get(TaskFuncPtr& task);

	//是否终止
	bool isTerminate()const { return m_bTerminate; }

	//线程运行态,取任务执行
	void run();

protected:
	//任务函数队列（存放一个个具体的任务函数）
	std::queue<TaskFuncPtr>		m_tasks;

	//工作线程
	std::vector<std::thread*>	m_pThreads;

	//任务队列互斥变量
	std::mutex m_mutex;

	//任务等待队列条件变量
	std::condition_variable		m_condition;

	//线程池线程数
	size_t						m_threadNum;

	//是否终止执行
	bool						m_bTerminate;

	//正在运行的任务数
	std::atomic<int>			m_atomic{ 0 };
};
#endif // !_CRC_THREAD_POOL_H_

