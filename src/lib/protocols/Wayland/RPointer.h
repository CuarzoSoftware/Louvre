#ifndef RPOINTER_H
#define RPOINTER_H

#include <LResource.h>
#include <LPointer.h>

class Louvre::Protocols::Wayland::RPointer : public LResource
{
public:
    RPointer(GSeat *gSeat, Int32 id);
    ~RPointer();

    GSeat *seatGlobal() const;

    // Since 1
    bool enter(const LPointerEnterEvent &event, RSurface *rSurface);
    bool leave(const LPointerLeaveEvent &event, RSurface *rSurface);
    bool motion(const LPointerMoveEvent &event);
    bool button(const LPointerButtonEvent &event);
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

    LPRIVATE_IMP_UNIQUE(RPointer)
};
#endif // RPOINTER_H
