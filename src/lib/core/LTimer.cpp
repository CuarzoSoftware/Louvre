#include <private/LTimerPrivate.h>
#include <LCompositor.h>

LTimer::LTimer(const Callback &onTimeout) : LPRIVATE_INIT_UNIQUE(LTimer)
{
    imp()->onTimeoutCallback = onTimeout;
    imp()->waylandEventSource = wl_event_loop_add_timer(LCompositor::eventLoop(), &LTimer::LTimerPrivate::waylandTimeoutCallback, this);
}

void LTimer::destroy()
{
    if (imp()->inCallback)
        imp()->pendingDestroy = true;
    else
        delete this;
}

LTimer::~LTimer()
{
    wl_event_source_timer_update(imp()->waylandEventSource, 0);
    wl_event_source_remove(imp()->waylandEventSource);
}

void LTimer::oneShot(UInt32 intervalMs, const Callback &onTimeout)
{
    if (!onTimeout)
        return;

    LTimer *tmpTimer = new LTimer(onTimeout);
    tmpTimer->start(intervalMs, true);
}

void LTimer::setCallback(const Callback &onTimeout)
{
    if (running())
        return;

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
        if (imp()->destroyOnTimeout)
        {
            if (imp()->inCallback)
                imp()->pendingDestroy = true;
            else
                delete this;
        }
        else
        {
            imp()->running = false;
            wl_event_source_timer_update(imp()->waylandEventSource, 0);
        }
    }
}

bool LTimer::start(UInt32 intervalMs, bool destroyOnTimout)
{
    if (running())
        return false;

    if (!imp()->onTimeoutCallback)
        return false;

    imp()->interval = intervalMs;
    imp()->running = true;
    imp()->destroyOnTimeout = destroyOnTimout;

    if (intervalMs > 0)
        wl_event_source_timer_update(imp()->waylandEventSource, intervalMs);
    else
        imp()->waylandTimeoutCallback(this);

    return true;
}
