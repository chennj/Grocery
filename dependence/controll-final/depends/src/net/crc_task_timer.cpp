#include "crc_task_timer.h"

#include "crc_task_timer.h"

void
CRCTaskTimer::add_task_1s(int id, int ms, Event e, int run_count)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    time_t  t1      = CRCTime::system_clock_now();
    Task    task    = { id, run_count , ms, t1, e };
    m_tasks.push_back(task);
}

void 
CRCTaskTimer::add_task(int id, int ms, Event e)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    time_t  t1      = CRCTime::system_clock_now();
    Task    task    = { id, -1, ms, t1, e };
    m_tasks.push_back(task);
}

void 
CRCTaskTimer::set_time(int id, int ms)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto itr = m_tasks.begin(); itr != m_tasks.end(); ++itr)
    {
        if (id == itr->id)
        {
            itr->ms = ms;
            return;
        }
    }
}

void 
CRCTaskTimer::stop_task(int id)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto itr = m_tasks.begin(); itr != m_tasks.end(); ++itr)
    {
        if (id == itr->id)
        {
            m_tasks.erase(itr);
            return;
        }
    }
}

void 
CRCTaskTimer::OnRun()
{
    //如果没有任务
    if (m_tasks.empty())
    {
        return;
    }
    //当前时间
    time_t d = CRCTime::system_clock_now();
    //处理任务
    for (auto itr = m_tasks.begin(); itr != m_tasks.end(); ++itr)
    {
        //(当前时间 - 起点时间) >= 间隔时间
        if ((d - itr->t1) >= itr->ms)
        {
            //更新计时起点时间
            itr->t1 = d; 
            //触发任务事件的回调方法
            itr->e();
            //如果剩余触发次数>0
            if (itr->run_count > 0)
            {	
                //触发次数-1
                --itr->run_count;
                //如果剩余次数为0
                if (itr->run_count == 0)
                {	
                    //移除任务
                    itr = m_tasks.erase(itr);
                }
                //判断是否最后一个成员
                if (itr == m_tasks.end())
                {
                    break;
                }
            }
        }
    }
}