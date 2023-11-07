#include <private/LTimerPrivate.h>

Int32 LTimer::LTimerPrivate::waylandTimeoutCallback(void *data)
{
    LTimer *timer = (LTimer*)data;

    timer->imp()->running = false;

    timer->imp()->inCallback = true;
    timer->imp()->onTimeoutCallback(timer);
    timer->imp()->inCallback = false;

    if (timer->imp()->pendingDestroy || timer->imp()->destroyOnTimeout)
        delete timer;

    return 0;
}
