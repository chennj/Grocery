#ifndef _CRC_THREAD_POOL_H_
#define _CRC_THREAD_POOL_H_

#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>>
#include <future>>

using namespace std;

#define TNOWMS 0 

class CRCThreadPool
{
protected:
	struct TaskFunc
	{
		TaskFunc(uint64_t expireTime):m_expireTime(expireTime)
		{}
		function<void()> m_func;
		int64_t m_expireTime = 0;
	};
	typedef shared_ptr<TaskFunc> TaskFuncPtr;

public:
	explicit CRCThreadPool();
	virtual ~CRCThreadPool();

public:
	//初始化
	//num：工作线程个数
	bool init(size_t num);

	//获取线程个数

	//返回一个执行任务的 future，可以通过这个对象获取返回值
	template<class F, class... Args>
	auto exec(int64_t timeoutMs, F&& f, Args&&... args)->future<decltype(f(args...))>
	{
		int64_t expireTime = (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);		//获取现在时间

		//返回返回值类型
		using RetType = decltype(f(args...));								//推导返回值

		//封装任务
		auto task make_shared<packaged_task<RetType()>>(bind(forward<F>f, forward<Args>(args)...));

		TaskFuncPtr fPtr = make_shared<TaskFunc>(expireTime);				//封装任务指针，设置过期时间
		fPtr->m_func = [task] {
			//具体执行的函数
			(*task)();
		};

		unique_lock<mutex> lock(m_mutex);
		m_tasks.push(fPtr);
		m_confition.notify_one();

		return task->get_future();
	}

	//等待当前任务队列中，所有工作全部结束
	bool waitForAllDone(int millsecond = -1);

	//启动线程池
	bool start();

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
	queue<TaskFuncPtr> m_tasks;

	//工作线程
	vector<thread*> m_pThreads;

	mutex m_mutex;

	condition_variable m_confition;

	size_t m_threadNum;

	bool m_bTerminate;

	atomic<int>	m_atomic{ 0 };
};
#endif // !_CRC_THREAD_POOL_H_

