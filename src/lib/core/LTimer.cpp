#include <private/LTimerPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LLog.h>

LTimer::LTimer(const Callback &onTimeout) : LPRIVATE_INIT_UNIQUE(LTimer)
{
    imp()->onTimeoutCallback = onTimeout;

    if (compositor()->display())
        imp()->waylandEventSource = wl_event_loop_add_timer(LCompositor::eventLoop(), &LTimer::LTimerPrivate::waylandTimeoutCallback, this);
}

LTimer::~LTimer()
{
    if (compositor()->display())
    {
        wl_event_source_timer_update(imp()->waylandEventSource, 0);
        wl_event_source_remove(imp()->waylandEventSource);
    }

    if (imp()->destroyOnTimeout)
        LVectorRemoveOneUnordered(compositor()->imp()->oneShotTimers, this);
}

void LTimer::oneShot(UInt32 intervalMs, const Callback &onTimeout)
{
    if (!onTimeout)
    {
        LLog::error("[LTimer::oneShot] Cannot create one shot LTimer without onTimeout callback.");
        return;
    }

    if (!compositor()->display())
    {
        LLog::error("[LTimer::oneShot] Failed to create one shot LTimer, no active LCompositor instance.");
        return;
    }

    LTimer *timer = new LTimer(onTimeout);
    timer->imp()->destroyOnTimeout = true;
    compositor()->imp()->oneShotTimers.push_back(timer);
    timer->start(intervalMs);
}

void LTimer::setCallback(const Callback &onTimeout)
{
    imp()->onTimeoutCallback = onTimeout;
}

UInt32 LTimer::interval() const
{
    return imp()->interval;
}

bool LTimer::running() const
{
    return imp()->running;
}

void LTimer::cancel()
{
    if (running())
    {
        imp()->running = false;

        if (compositor()->display())
            wl_event_source_timer_update(imp()->waylandEventSource, 0);

        if (imp()->destroyOnTimeout)
        {
            delete this;
            return;
        }
    }
}

void LTimer::stop()
{
    if (running())
    {
        imp()->running = false;

        if (compositor()->display())
            wl_event_source_timer_update(imp()->waylandEventSource, 0);

        if (imp()->onTimeoutCallback)
            imp()->onTimeoutCallback(this);

        if (!imp()->running && imp()->destroyOnTimeout)
        {
            delete this;
            return;
        }
    }
}

void LTimer::start(UInt32 intervalMs)
{
    if (!compositor()->display())
    {
        LLog::error("[LTimer::oneShot] Failed to start LTimer, no active LCompositor instance.");
        return;
    }

    if (!imp()->waylandEventSource)
        imp()->waylandEventSource = wl_event_loop_add_timer(LCompositor::eventLoop(), &LTimer::LTimerPrivate::waylandTimeoutCallback, this);

    imp()->interval = intervalMs;
    imp()->running = true;

    if (intervalMs > 0)
        wl_event_source_timer_update(imp()->waylandEventSource, intervalMs);
    else
        imp()->waylandTimeoutCallback(this);
}
