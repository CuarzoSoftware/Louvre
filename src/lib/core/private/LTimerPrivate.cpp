#include <private/LTimerPrivate.h>

Int32 LTimer::LTimerPrivate::waylandTimeoutCallback(void *data)
{
    LTimer *timer = (LTimer*)data;

    timer->imp()->running = false;

    timer->imp()->onTimeoutCallback(timer);

    if (timer->imp()->destroyOnTimeout)
        delete timer;

    return 0;
}
