#include "crc_timer.h"

CRCTimer::CRCTimer(uint32_t id, int64_t when_ms, int64_t interval_ms, const TimerTask& handler)
    : m_interval_ms(interval_ms)
    , m_repeated(m_interval_ms > 0)
    , m_when_ms(when_ms)
    , m_id(id)
    , m_timer_task(handler)
{
}

void
CRCTimer::Run()
{
    if (m_timer_task){
        m_timer_task();
    }
}