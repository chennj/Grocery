#include "crc_thread_pool.h"
#include "crc_log.h"

#include <iostream>

int64_t getNowMs()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

CRCThreadPool::CRCThreadPool()
	:m_threadNum(1),m_bTerminate(false)
{
}

CRCThreadPool::~CRCThreadPool()
{
	stop();
}

bool CRCThreadPool::init(size_t num)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (!m_pThreads.empty())
	{
		return false;
	}

	m_threadNum = num;
	return true;
}

size_t CRCThreadPool::getThreadNum()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_pThreads.size();
}

size_t CRCThreadPool::getJobNum()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_tasks.size();
}

bool CRCThreadPool::waitForAllDone(int millsecond)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_tasks.empty()) {
		return true;
	}

	if (millsecond < 0) {
		m_condition.wait(lock, [this] {return m_tasks.empty(); });
		return true;
	}
	else {
		return m_condition.wait_for(lock, std::chrono::milliseconds(millsecond), [this] {return m_tasks.empty(); });
	}
}

bool CRCThreadPool::start()
{
	std::unique_lock <std::mutex> lock(m_mutex);

	if (!m_pThreads.empty()) {
		return false;
	}

	for (size_t i = 0; i < m_threadNum; i++)
	{
		//下面传递 this 是为了可以开启多个线程池
		m_pThreads.push_back(new std::thread(&CRCThreadPool::run, this));
	}

	return true;
}

void CRCThreadPool::stop()
{
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_bTerminate = true;
		m_condition.notify_all();
	}

	for (size_t i = 0; i < m_pThreads.size(); i++)
	{
		if (m_pThreads[i]->joinable())
		{
			m_pThreads[i]->join();
		}
		delete m_pThreads[i];
		m_pThreads[i] = NULL;
	}

	std::unique_lock<std::mutex> lock(m_mutex);
	m_pThreads.clear();
}

bool CRCThreadPool::get(TaskFuncPtr& task)
{
	std::unique_lock<std::mutex> lock(m_mutex);

	if (m_tasks.empty()) {
		m_condition.wait(lock, [this] {return m_bTerminate || !m_tasks.empty(); });
	}

	if (m_bTerminate) return false;

	if (!m_tasks.empty()) 
	{
		task = move(m_tasks.front());
		m_tasks.pop();
		return true;
	}

	return false;
}

void CRCThreadPool::run()
{
	//调用处理部分
	while (!isTerminate())
	{
		TaskFuncPtr task;
		bool ok = get(task);
		if (ok)
		{
			++m_atomic;
			try
			{
				if (task->m_expireTime != 0 && task->m_expireTime < TNOWMS)
				{
					//超时任务，是否需要处理
					//std::cout << "has task timeout: " << task << std::endl;
                    CRCLog_Warring("thread pool has task timeout");
				}
				else
				{
					task->m_func();	//执行任务
				}
			}
			catch (const std::exception& e)
			{
				//std::cout << "exception: " << e.what() << std::endl;
                CRCLog_Warring("thread pool has task exception %s",e.what());
			}

			--m_atomic;

			//任务执行完毕了
			std::unique_lock<std::mutex> lock(m_mutex);
			if (m_atomic == 0 && m_tasks.empty())
			{
				//通知 waitforalldone
				m_condition.notify_all();
			}
		}
	}
}
