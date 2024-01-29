#include <private/LTimerPrivate.h>
#include <LCompositor.h>

LTimer::LTimer(const Callback &onTimeout) : LPRIVATE_INIT_UNIQUE(LTimer)
{
    imp()->onTimeoutCallback = onTimeout;
    imp()->waylandEventSource = wl_event_loop_add_timer(LCompositor::eventLoop(), &LTimer::LTimerPrivate::waylandTimeoutCallback, this);
}

LTimer::~LTimer()
{
    stop();
    wl_event_source_timer_update(imp()->waylandEventSource, 0);
    wl_event_source_remove(imp()->waylandEventSource);
}

void LTimer::oneShot(UInt32 intervalMs, const Callback &onTimeout)
{
    if (!onTimeout)
        return;

    LTimer *timer = new LTimer(onTimeout);
    timer->imp()->destroyOnTimeout = true;
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
    imp()->interval = intervalMs;
    imp()->running = true;

    if (intervalMs > 0)
        wl_event_source_timer_update(imp()->waylandEventSource, intervalMs);
    else
        imp()->waylandTimeoutCallback(this);
}
