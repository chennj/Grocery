#ifndef _CRC_TIME_WHEEL_H_
#define _CRC_TIME_WHEEL_H_

#include <chrono>
#include <string>
#include <memory>
#include <vector>
#include <list>

#include "crc_timer.h"

class CRCTimeWheel
{
private:
    std::string m_name;
    uint32_t    m_current_index;

    // A time wheel can be devided into multiple scales. A scals has N ms.
    uint32_t m_scales;
    uint32_t m_scale_unit_ms;

    // Every slot corresponds to a scale. Every slot contains the timers.
    std::vector<std::list<CRCTimerPtr> > m_slots;

    CRCTimeWheel* m_less_level_tw;     // Less scale unit.
    CRCTimeWheel* m_greater_level_tw;  // Greater scale unit.

public:
    CRCTimeWheel(uint32_t scales, uint32_t scale_unit_ms, const std::string& name = "");

    uint32_t scale_unit_ms() const {
        return m_scale_unit_ms;
    }

    uint32_t scales() const {
        return m_scales;
    }

    uint32_t current_index() const {
        return m_current_index;
    }

    void set_less_level_tw(CRCTimeWheel* less_level_tw) {
        m_less_level_tw = less_level_tw;
    }

    void set_greater_level_tw(CRCTimeWheel* greater_level_tw) {
        m_greater_level_tw = greater_level_tw;
    }

    int64_t GetCurrentTime() const;

    void AddTimer(CRCTimerPtr timer);

    void Increase();

    std::list<CRCTimerPtr> GetAndClearCurrentSlot();
};

using CRCTimeWheelPtr = std::shared_ptr<CRCTimeWheel>;

#endif //!_CRC_TIME_WHEEL_H_