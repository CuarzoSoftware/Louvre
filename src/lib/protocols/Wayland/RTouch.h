#ifndef RTOUCH_H
#define RTOUCH_H

#include <LResource.h>
#include <LTouchDownEvent.h>
#include <LTouchUpEvent.h>

class Louvre::Protocols::Wayland::RTouch : public LResource
{
public:
    RTouch(GSeat *gSeat, Int32 id);
    ~RTouch();

    GSeat *seatGlobal() const;

    // Since 1
    bool down(const LTouchDownEvent &event, RSurface *rSurface);
    bool up(const LTouchUpEvent &event);
    bool motion(UInt32 time, Int32 id, Float24 x, Float24 y);
    bool frame();
    bool cancel();

    // Since 6
    bool shape(Int32 id, Float24 major, Float24 minor);
    bool orientation(Int32 id, Float24 orientation);

    LPRIVATE_IMP_UNIQUE(RTouch)
};

#endif // RTOUCH_H
