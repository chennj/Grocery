#ifndef _CRC_TASK_TIMER_H_
#define _CRC_TASK_TIMER_H_

#include<mutex>
#include<list>
#include<functional>

#include "crc_timestamp.h"

class CRCTaskTimer
{
private:
    //事件
    typedef std::function<void()> Event;
    //任务结构
    struct Task
    {
        //任务的id
        int id;
        //事件执行次数
        int run_count;
        //执行事件的间隔事件
        time_t ms;
        //创建任务的时间点
        time_t t1;
        //要执行的事件
        Event e;
    };
private:
    //任务数据
    std::list<Task> m_tasks;
    //改变数据缓冲区时需要加锁
    std::mutex      m_mutex;
public:
    //run_count次定时触发任务事件
    void add_task_1s(int id, int ms, Event e, int run_count = 1);

    //无限次定时触发任务事件
    void add_task(int id, int ms, Event e);

    //设置定时任务的间隔时间
    void set_time(int id, int ms);

    //设置定时任务的间隔时间
    void stop_task(int id);

    //工作函数
    void OnRun();
};

#endif //!_CRC_TASK_TIMER_H_