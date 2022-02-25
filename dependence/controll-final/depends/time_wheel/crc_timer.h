/**
 * 
 * author:  chenningjiang
 * desc:    定时器实体对象
 * 
 * */
#ifndef _CRC_TIMER_H_
#define _CRC_TIMER_H_

#include <cstdint>
#include <functional>
#include <memory>

typedef std::function<void()> TimerTask;

class CRCTimer
{
private:
    uint32_t    m_id;
    TimerTask   m_timer_task;
    int64_t     m_when_ms;
    uint32_t    m_interval_ms;
    bool        m_repeated;

public:
    CRCTimer(uint32_t id, int64_t when_ms, int64_t interval_ms, const TimerTask& handler);

public:

    uint32_t id() const {
        return m_id;
    }

    int64_t when_ms() const {
        return m_when_ms;
    }

    bool repeated() const {
        return m_repeated;
    }

    void UpdateWhenTime() {
        m_when_ms += m_interval_ms;
    }

    void Run();
};

using CRCTimerPtr = std::shared_ptr<CRCTimer>;

#endif //!_CRC_TIMER_H_