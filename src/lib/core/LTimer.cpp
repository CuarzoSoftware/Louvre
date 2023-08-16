#include <private/LTimerPrivate.h>
#include <LCompositor.h>

LTimer::LTimer(const Callback &onTimeout)
{
    m_imp = new LTimerPrivate();
    imp()->onTimeoutCallback = onTimeout;
    imp()->waylandEventSource = wl_event_loop_add_timer(LCompositor::eventLoop(), &LTimer::LTimerPrivate::waylandTimeoutCallback, this);
}

LTimer::~LTimer()
{
    wl_event_source_timer_update(imp()->waylandEventSource, 0);
    wl_event_source_remove(imp()->waylandEventSource);
    delete m_imp;
}

void LTimer::oneShot(UInt32 intervalMs, const Callback &onTimeout)
{
    LTimer *tmpTimer = new LTimer(onTimeout);
    tmpTimer->start(intervalMs, true);
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
        if (imp()->destroyOnTimeout)
            delete this;
        else
        {
            imp()->running = false;
            wl_event_source_timer_update(imp()->waylandEventSource, 0);
        }
    }
}

void LTimer::start(UInt32 intervalMs, bool destroyOnTimout)
{
    if (running())
        return;

    imp()->interval = intervalMs;
    imp()->running = true;
    imp()->destroyOnTimeout = destroyOnTimout;

    if (intervalMs > 0)
        wl_event_source_timer_update(imp()->waylandEventSource, intervalMs);
    else
    {
        imp()->waylandTimeoutCallback(this);
    }
}
