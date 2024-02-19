#ifndef RGESTUREHOLD_H
#define RGESTUREHOLD_H

#include <LResource.h>
#include <LPointerHoldBeginEvent.h>
#include <LPointerHoldEndEvent.h>

class Louvre::Protocols::PointerGestures::RGestureHold : public LResource
{
public:
    RGestureHold(Protocols::Wayland::RPointer *rPointer, Int32 id, UInt32 version);
    LCLASS_NO_COPY(RGestureHold)
    ~RGestureHold();

    struct SerialEvents
    {
        LPointerHoldBeginEvent begin;
        LPointerHoldEndEvent end;
    };

    Protocols::Wayland::RPointer *pointerResource() const;
    const SerialEvents &serialEvents() const;

    // Since 1
    bool begin(const LPointerHoldBeginEvent &event, Protocols::Wayland::RSurface *rSurface);
    bool end(const LPointerHoldEndEvent &event);

    LPRIVATE_IMP_UNIQUE(RGestureHold);
};

#endif // RGESTUREHOLD_H
