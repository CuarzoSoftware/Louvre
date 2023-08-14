#include <private/LTimerPrivate.h>

Int32 LTimer::LTimerPrivate::waylandTimeoutCallback(void *data)
{
    LTimer *timer = (LTimer*)data;
    timer->imp()->onTimeoutCallback(timer);
}
