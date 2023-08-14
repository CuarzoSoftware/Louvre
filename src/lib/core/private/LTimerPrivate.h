#ifndef LTIMERPRIVATE_H
#define LTIMERPRIVATE_H

#include <LTimer.h>

using namespace Louvre;

LPRIVATE_CLASS(LTimer)

LTimer::Callback onTimeoutCallback;
wl_event_source *waylandEventSource = nullptr;
static Int32 waylandTimeoutCallback(void *data);

};

#endif // LTIMERPRIVATE_H
