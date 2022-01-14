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
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/utime.h>
#endif

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
	//��ʼ��
	//num�������̸߳���
	bool init(size_t num);

	//��ȡ�̸߳���
	size_t getThreadNum();

	//��ȡ��ǰ�̳߳ص�������
	size_t getJobNum();

	//����һ��ִ������� future������ͨ����������ȡ����ֵ
	//future<decltype(f(args...))> ����future�������߿���ͨ��future��ȡ����ֵ
	template<class F, class... Args>
	auto exec(int64_t timeoutMs, F&& f, Args&&... args)->std::future<decltype(f(args...))>
	{
		int64_t expireTime = (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);		//��ȡ����ʱ��

		//���ط���ֵ����
		using RetType = decltype(f(args...));								//�Ƶ�����ֵ

		//��װ����
		auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		TaskFuncPtr fPtr = std::make_shared<TaskFunc>(expireTime);			//��װ����ָ�룬���ù���ʱ��
		fPtr->m_func = [task]() {
			//����ִ�еĺ���
			(*task)();
		};

		std::unique_lock<std::mutex> lock(m_mutex);
		m_tasks.push(fPtr);			//��������
		m_condition.notify_one();	//�����������̣߳����Կ���ֻ��������в�Ϊ�յ������ȥnotify

		return task->get_future();
	}

	//����һ��ִ������� future������ͨ����������ȡ����ֵ
	template<class F, class... Args>
	auto exec(F&& f, Args&&... args)->std::future<decltype(f(args...))>
	{
		return exec(0, f, args...);
	}

	//�ȴ���ǰ��������У����й���ȫ������������������
	//millsecond	-1:��Զ�ȴ�
	//����			true, ���й������������
	//				false,��ʱ�˳�
	bool waitForAllDone(int millsecond = -1);

	//�����̳߳�
	bool start();

	void stop();

protected:
	//��ȡ��������е�����ִ�к���
	bool get(TaskFuncPtr& task);

	//�Ƿ���ֹ
	bool isTerminate()const { return m_bTerminate; }

	//�߳�����̬,ȡ����ִ��
	void run();

protected:
	//���������У����һ�����������������
	std::queue<TaskFuncPtr>		m_tasks;

	//�����߳�
	std::vector<std::thread*>	m_pThreads;

	std::mutex m_mutex;

	std::condition_variable		m_condition;

	size_t						m_threadNum;

	bool						m_bTerminate;

	std::atomic<int>			m_atomic{ 0 };
};
#endif // !_CRC_THREAD_POOL_H_

