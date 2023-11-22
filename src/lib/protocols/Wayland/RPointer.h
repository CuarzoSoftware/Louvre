#ifndef RPOINTER_H
#define RPOINTER_H

#include <LResource.h>
#include <LPointer.h>

class Louvre::Protocols::Wayland::RPointer : public LResource
{
public:
    RPointer(GSeat *gSeat, Int32 id);
    ~RPointer();

    struct LastEventSerials
    {
        UInt32 leave = 0;
        UInt32 enter = 0;
        UInt32 button = 0;
    };

    GSeat *seatGlobal() const;
    const LastEventSerials &serials() const;

    // Since 1
    bool enter(UInt32 serial, RSurface *rSurface, Float24 x, Float24 y);
    bool leave(UInt32 serial, RSurface *rSurface);
    bool motion(UInt32 time, Float24 x, Float24 y);
    bool button(UInt32 serial, UInt32 time, UInt32 button, UInt32 state);
    bool axis(UInt32 time, UInt32 axis, Float24 value);

    // Since 5
    bool frame();
    bool axisSource(UInt32 axisSource);
    bool axisStop(UInt32 time, UInt32 axis);
    bool axisDiscrete(UInt32 axis, Int32 discrete);

    // Since 8
    bool axisValue120(UInt32 axis, Int32 value120);

    // Since 9
    bool axisRelativeDirection(UInt32 axis, UInt32 direction);

    LPRIVATE_IMP_UNIQUE(RPointer);
};
#endif // RPOINTER_H
