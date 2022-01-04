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
	//��ʼ��
	//num�������̸߳���
	bool init(size_t num);

	//��ȡ�̸߳���

	//����һ��ִ������� future������ͨ����������ȡ����ֵ
	template<class F, class... Args>
	auto exec(int64_t timeoutMs, F&& f, Args&&... args)->future<decltype(f(args...))>
	{
		int64_t expireTime = (timeoutMs == 0 ? 0 : TNOWMS + timeoutMs);		//��ȡ����ʱ��

		//���ط���ֵ����
		using RetType = decltype(f(args...));								//�Ƶ�����ֵ

		//��װ����
		auto task make_shared<packaged_task<RetType()>>(bind(forward<F>f, forward<Args>(args)...));

		TaskFuncPtr fPtr = make_shared<TaskFunc>(expireTime);				//��װ����ָ�룬���ù���ʱ��
		fPtr->m_func = [task] {
			//����ִ�еĺ���
			(*task)();
		};

		unique_lock<mutex> lock(m_mutex);
		m_tasks.push(fPtr);
		m_confition.notify_one();

		return task->get_future();
	}

	//�ȴ���ǰ��������У����й���ȫ������
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
	queue<TaskFuncPtr> m_tasks;

	//�����߳�
	vector<thread*> m_pThreads;

	mutex m_mutex;

	condition_variable m_confition;

	size_t m_threadNum;

	bool m_bTerminate;

	atomic<int>	m_atomic{ 0 };
};
#endif // !_CRC_THREAD_POOL_H_

