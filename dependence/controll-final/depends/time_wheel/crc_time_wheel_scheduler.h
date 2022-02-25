#ifndef _CRC_TIME_WHEEL_SCHEDULER_H_
#define _CRC_TIME_WHEEL_SCHEDULER_H_

#include <mutex>
#include <vector>
#include <thread>
#include <unordered_set>

#include "crc_time_wheel.h"

class CRCTimeWheelScheduler
{
public:
    explicit CRCTimeWheelScheduler(uint32_t timer_step_ms = 50);

    // Return timer id. Return 0 if the timer creation fails.
    uint32_t CreateTimerAt(int64_t when_ms, const TimerTask& task);
    uint32_t CreateTimerAfter(int64_t delay_ms, const TimerTask& task);
    uint32_t CreateTimerEvery(int64_t interval_ms, const TimerTask& task);

    void CancelTimer(uint32_t timer_id);

    bool Start();
    void Stop();

    void AppendTimeWheel(uint32_t scales, uint32_t scale_unit_ms, const std::string& name = "");

    private:
    void Run();

    CRCTimeWheelPtr GetGreatestTimeWheel();
    CRCTimeWheelPtr GetLeastTimeWheel();

    private:
    std::mutex  m_mutex;
    std::thread m_thread;

    bool m_stop;

    std::unordered_set<uint32_t>    m_cancel_timer_ids;

    uint32_t                        m_timer_step_ms;
    std::vector<CRCTimeWheelPtr>    m_time_wheels;
};

#endif //!_CRC_TIME_WHEEL_SCHEDULER_H_