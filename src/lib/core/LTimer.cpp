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

}

void LTimer::oneShot(UInt32 intervalMs, const Callback &onTimeout)
{

}

void LTimer::setCallback(const Callback &onTimeout)
{

}

UInt32 LTimer::interval() const
{

}

bool LTimer::running() const
{

}

void LTimer::cancel()
{

}

void LTimer::start(UInt32 interval)
{
    //wl_event_source_timer_update(clockTimer, 1);
}
