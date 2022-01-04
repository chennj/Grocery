#include "CRCThreadPool.h"

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
	unique_lock<mutex> lock(m_mutex);

	if (!m_pThreads.empty())
	{
		return false;
	}

	m_threadNum = num;
	return true;
}

bool CRCThreadPool::start()
{
	unique_lock <mutex> lock(m_mutex);

	if (!m_pThreads.empty()) {
		return false;
	}

	for (size_t i = 0; i < m_threadNum; i++)
	{
		//���洫�� this ��Ϊ�˿��Կ�������̳߳�
		m_pThreads.push_back(new thread(&CRCThreadPool::run, this));
	}

	return true;
}

void CRCThreadPool::stop()
{
	{
		unique_lock<mutex> lock(m_mutex);
		m_bTerminate = true;
		m_confition.notify_all();
	}

	for (size_t i = 0; i < m_threadNum; i++)
	{
		if (m_pThreads[i]->joinable())
		{
			m_pThreads[i]->join();
		}
		delete m_pThreads[i];
		m_pThreads[i] = NULL;
	}
}

bool CRCThreadPool::get(TaskFuncPtr & task)
{
	unique_lock<mutex> lock(m_mutex);

	if (m_tasks.empty()) {
		m_confition.wait(lock, [this] {return m_bTerminate || !m_tasks.empty(); });
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
	//���ô�����
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
					//���������Ƿ���Ҫ����
				}
				else
				{
					task->m_func;	//ִ������
				}
			}
			catch (const std::exception&)
			{

			}

			--m_atomic;

			//����ִ�������
			unique_lock<mutex> lock(m_mutex);
			if (m_atomic == 0 && m_tasks.empty())
			{
				//֪ͨ waitforalldone
				m_confition.notify_all();
			}
		}
	}
}
