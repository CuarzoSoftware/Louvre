#ifndef LWAYLANDKEYBOARDRESOURCE_H
#define LWAYLANDKEYBOARDRESOURCE_H

#include <LResource.h>

using namespace Louvre;

class LWaylandSeatGlobal;

class LWaylandKeyboardResource : public LResource
{
public:
    LWaylandKeyboardResource(LWaylandSeatGlobal *seatGlobal, Int32 id);
    ~LWaylandKeyboardResource();

    // Events
    void sendRepeatInfo(Int32 rate, Int32 delay);
    void sendKeymap(Int32 fd, UInt32 size);
    void sendLeave(LSurface *surface);
    void sendEnter(LSurface *surface);
    void sendModifiers(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group);
    void sendKey(UInt32 key, UInt32 state);

    LWaylandSeatGlobal *seatGlobal() const;

LPRIVATE_IMP(LWaylandKeyboardResource)
};

#endif // LWAYLANDKEYBOARDRESOURCE_H
