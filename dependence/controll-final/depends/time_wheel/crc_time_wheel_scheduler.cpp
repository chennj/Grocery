#include "crc_time_wheel_scheduler.h"
#include "crc_timestamp.h"

static uint32_t s_inc_id = 1;

CRCTimeWheelScheduler::CRCTimeWheelScheduler(uint32_t timer_step_ms)
    : m_timer_step_ms(timer_step_ms)
    , m_stop(false) {
}

bool 
CRCTimeWheelScheduler::Start() 
{
    if (m_timer_step_ms < 50) {
        return false;
    }

    if (m_time_wheels.empty()) {
        return false;
    }

    m_thread = std::thread(std::bind(&CRCTimeWheelScheduler::Run, this));

    return true;
}

void 
CRCTimeWheelScheduler::Run() 
{
    while (true) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(m_timer_step_ms));

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stop) {
            break;
        }

        CRCTimeWheelPtr least_time_wheel = GetLeastTimeWheel();
        least_time_wheel->Increase();
        std::list<CRCTimerPtr> slot = std::move(least_time_wheel->GetAndClearCurrentSlot());
        for (const CRCTimerPtr& timer : slot) {
            auto it = m_cancel_timer_ids.find(timer->id());
            if (it != m_cancel_timer_ids.end()) {
                m_cancel_timer_ids.erase(it);
                continue;
            }

            timer->Run();
            if (timer->repeated()) {
                timer->UpdateWhenTime();
                GetGreatestTimeWheel()->AddTimer(timer);
            }
        }
    }
}

void 
CRCTimeWheelScheduler::Stop() 
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }

    m_thread.join();
}

CRCTimeWheelPtr 
CRCTimeWheelScheduler::GetGreatestTimeWheel() 
{
    if (m_time_wheels.empty()) {
        return CRCTimeWheelPtr();
    }

    return m_time_wheels.front();
}

CRCTimeWheelPtr 
CRCTimeWheelScheduler::GetLeastTimeWheel() 
{
    if (m_time_wheels.empty()) {
        return CRCTimeWheelPtr();
    }

    return m_time_wheels.back();
}

void 
CRCTimeWheelScheduler::AppendTimeWheel(uint32_t scales, uint32_t scale_unit_ms, const std::string& name) 
{
    CRCTimeWheelPtr time_wheel = std::make_shared<CRCTimeWheel>(scales, scale_unit_ms, name);
    if (m_time_wheels.empty()) {
        m_time_wheels.push_back(time_wheel);
        return;
    }

    CRCTimeWheelPtr greater_time_wheel = m_time_wheels.back();
    greater_time_wheel->set_less_level_tw(time_wheel.get());
    time_wheel->set_greater_level_tw(greater_time_wheel.get());
    m_time_wheels.push_back(time_wheel);
}

uint32_t 
CRCTimeWheelScheduler::CreateTimerAt(int64_t when_ms, const TimerTask& task) 
{
    if (m_time_wheels.empty()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    ++s_inc_id;
    GetGreatestTimeWheel()->AddTimer(std::make_shared<CRCTimer>(s_inc_id, when_ms, 0, task));

    return s_inc_id;
}

uint32_t 
CRCTimeWheelScheduler::CreateTimerAfter(int64_t delay_ms, const TimerTask& task) 
{
    int64_t when = CRCTime::system_clock_now() + delay_ms;
    return CreateTimerAt(when, task);
}

uint32_t 
CRCTimeWheelScheduler::CreateTimerEvery(int64_t interval_ms, const TimerTask& task) 
{
    if (m_time_wheels.empty()) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    ++s_inc_id;
    int64_t when = CRCTime::system_clock_now() + interval_ms;
    GetGreatestTimeWheel()->AddTimer(std::make_shared<CRCTimer>(s_inc_id, when, interval_ms, task));

    return s_inc_id;
}

void 
CRCTimeWheelScheduler::CancelTimer(uint32_t timer_id) 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cancel_timer_ids.insert(timer_id);
}