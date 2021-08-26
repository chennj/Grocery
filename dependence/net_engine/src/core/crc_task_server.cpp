#include "../../include/core/crc_task_server.h"

void
CRCTaskServer::addTask(CRCTask task)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _tasksBuf.push_back(task);
}

void
CRCTaskServer::Start()
{
    _thread.Start(nullptr, [this](CRCThread* pThread) {
        OnRun(pThread);
    },nullptr);
}

void
CRCTaskServer::Close()
{
    _thread.Close();
}

void
CRCTaskServer::OnRun(CRCThread* pThread)
{
    while (pThread->isRun())
    {
        //从缓冲区取出数据
        if (!_tasksBuf.empty())
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (auto pTask : _tasksBuf)
            {
                _tasks.push_back(pTask);
            }
            _tasksBuf.clear();
        }
        //如果没有任务
        if (_tasks.empty())
        {
            CRCThread::Sleep(1);
            continue;
        }
        //处理任务
        for (auto pTask : _tasks)
        {
            pTask();
        }
        //清空任务
        _tasks.clear();
    }
    //处理缓冲队列中的任务
    for (auto pTask : _tasksBuf)
    {
        pTask();
    }
}