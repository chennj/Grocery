#include "../../include/crc_thread.h"

void
CRCThread::Sleep(time_t dt)
{
    std::chrono::milliseconds t(dt);
    std::this_thread::sleep_for(t);
}

void
CRCThread::Start(EventCall onCreate, EventCall onRun, EventCall onDestory)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!_isRun)
    {
        _isRun = true;

        if (onCreate)
            _onCreate = onCreate;
        if (onRun)
            _onRun = onRun;
        if (onDestory)
            _onDestory = onDestory;

        //线程
        std::thread t(std::mem_fn(&CRCThread::OnWork), this);
        t.detach();
    }
}

void
CRCThread::Close()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_isRun)
    {
        _isRun = false;
        _sem.wait();
    }
}

void
CRCThread::Exit()
{
    if (_isRun)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _isRun = false;
    }
}

void
CRCThread::OnWork()
{
    if (_onCreate)
        _onCreate(this);
    if (_onRun)
        _onRun(this);
    if (_onDestory)
        _onDestory(this);

    _sem.wakeup();
    _isRun = false;
}
