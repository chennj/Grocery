#include "crc_time_wheel.h"
#include "crc_timestamp.h"

CRCTimeWheel::CRCTimeWheel(uint32_t scales, uint32_t scale_unit_ms, const std::string& name)
    : m_name(name)
    , m_current_index(0)
    , m_scales(scales)
    , m_scale_unit_ms(scale_unit_ms)
    , m_slots(scales)
    , m_greater_level_tw(nullptr)
    , m_less_level_tw(nullptr) {
}

int64_t 
CRCTimeWheel::GetCurrentTime() const 
{
    int64_t time = m_current_index * m_scale_unit_ms;
    if (m_less_level_tw != nullptr) {
        time += m_less_level_tw->GetCurrentTime();
    }

    return time;
}

void 
CRCTimeWheel::AddTimer(CRCTimerPtr timer) 
{
    int64_t less_tw_time = 0;
    if (m_less_level_tw != nullptr) {
        less_tw_time = m_less_level_tw->GetCurrentTime();
    }
    int64_t diff = timer->when_ms() + less_tw_time - CRCTime::system_clock_now();

    // If the difference is greater than scale unit, the timer can be added into the current time wheel.
    if (diff >= m_scale_unit_ms) {
        size_t n = (m_current_index + diff / m_scale_unit_ms) % m_scales;
        m_slots[n].push_back(timer);
        return;
    }

    // If the difference is less than scale uint, the timer should be added into less level time wheel.
    if (m_less_level_tw != nullptr) {
        m_less_level_tw->AddTimer(timer);
        return;
    }

    // If the current time wheel is the least level, the timer can be added into the current time wheel.
    m_slots[m_current_index].push_back(timer);
}

void 
CRCTimeWheel::Increase() 
{
    // Increase the time wheel.
    ++m_current_index;
    if (m_current_index < m_scales) {
        return;
    }

    // If the time wheel is full, the greater level time wheel should be increased.
    // The timers in the current slot of the greater level time wheel should be moved into
    // the less level time wheel.
    m_current_index = m_current_index % m_scales;
    if (m_greater_level_tw != nullptr) {
        m_greater_level_tw->Increase();
        std::list<CRCTimerPtr> slot = std::move(m_greater_level_tw->GetAndClearCurrentSlot());
        for (CRCTimerPtr timer : slot) {
            AddTimer(timer);
        }
    }
}

std::list<CRCTimerPtr> 
CRCTimeWheel::GetAndClearCurrentSlot() 
{
    std::list<CRCTimerPtr> slot;
    slot = std::move(m_slots[m_current_index]);
    return slot;
}