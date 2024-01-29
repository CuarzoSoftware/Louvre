#ifndef LTIMERPRIVATE_H
#define LTIMERPRIVATE_H

#include <LTimer.h>

using namespace Louvre;

LPRIVATE_CLASS(LTimer)

UInt32 interval = 0;
bool running = false;
bool destroyOnTimeout = false;
bool pendingDestroy = false;
LTimer::Callback onTimeoutCallback;
wl_event_source *waylandEventSource = nullptr;

inline static Int32 waylandTimeoutCallback(void *data)
{
    LTimer *timer = (LTimer*)data;
    timer->stop();
    return 0;
}
};

#endif // LTIMERPRIVATE_H
