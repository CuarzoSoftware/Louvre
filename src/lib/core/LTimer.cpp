#include <private/LCompositorPrivate.h>
#include <LTimer.h>
#include <LLog.h>

LTimer::LTimer(const Callback &onTimeout) noexcept : m_onTimeoutCallback(onTimeout)
{
    if (compositor() && compositor()->display())
        m_waylandEventSource = wl_event_loop_add_timer(LCompositor::eventLoop(), &LTimer::waylandTimeoutCallback, this);
}

LTimer::~LTimer() noexcept
{
    if (compositor()->display())
    {
        wl_event_source_timer_update(m_waylandEventSource, 0);
        wl_event_source_remove(m_waylandEventSource);
    }

    if (m_destroyOnTimeout)
        LVectorRemoveOneUnordered(compositor()->imp()->oneShotTimers, this);
}

bool LTimer::oneShot(UInt32 intervalMs, const Callback &onTimeout) noexcept
{
    if (!onTimeout)
    {
        LLog::error("[LTimer::oneShot] Cannot create one shot LTimer without onTimeout callback.");
        return false;
    }

    if (!compositor() || !compositor()->display())
    {
        LLog::error("[LTimer::oneShot] Failed to create one shot LTimer, no active LCompositor instance.");
        return false;
    }

    LTimer *timer { new LTimer(onTimeout) };
    timer->m_destroyOnTimeout = true;
    compositor()->imp()->oneShotTimers.push_back(timer);
    timer->start(intervalMs);
    return true;
}

void LTimer::cancel() noexcept
{
    if (running())
    {
        m_running = false;

        if (compositor() && compositor()->display())
            wl_event_source_timer_update(m_waylandEventSource, 0);

        if (m_destroyOnTimeout)
        {
            delete this;
            return;
        }
    }
}

void LTimer::stop() noexcept
{
    if (running())
    {
        m_running = false;

        if (compositor() && compositor()->display())
            wl_event_source_timer_update(m_waylandEventSource, 0);

        if (m_onTimeoutCallback)
            m_onTimeoutCallback(this);

        if (!m_running && m_destroyOnTimeout)
        {
            delete this;
            return;
        }
    }
}

bool LTimer::start(UInt32 intervalMs) noexcept
{
    if (!compositor() || !compositor()->display())
    {
        LLog::error("[LTimer::oneShot] Failed to start LTimer, no active LCompositor instance.");
        return false;
    }

    if (!m_onTimeoutCallback)
        return false;

    if (!m_waylandEventSource)
        m_waylandEventSource = wl_event_loop_add_timer(LCompositor::eventLoop(), &LTimer::waylandTimeoutCallback, this);

    m_interval = intervalMs;
    m_running = true;

    if (intervalMs > 0)
        wl_event_source_timer_update(m_waylandEventSource, intervalMs);
    else
        waylandTimeoutCallback(this);

    return true;
}

Int32 LTimer::waylandTimeoutCallback(void *data) noexcept
{
    static_cast<LTimer*>(data)->stop();
    return 0;
}
