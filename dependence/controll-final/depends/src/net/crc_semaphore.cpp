#include "crc_semaphore.h"

void 
CRCSemaphore::wait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if (--_wait < 0)
    {
        //阻塞等待
        _cv.wait(lock, [this]()->bool{
            return _wakeup > 0;
        });
        --_wakeup;
    }
}

void 
CRCSemaphore::wakeup()
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (++_wait <= 0)
    {
        ++_wakeup;
        _cv.notify_one();
    }
}
