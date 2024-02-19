#ifndef RGESTURESWIPE_H
#define RGESTURESWIPE_H

#include <LResource.h>
#include <LPointerSwipeBeginEvent.h>
#include <LPointerSwipeEndEvent.h>

class Louvre::Protocols::PointerGestures::RGestureSwipe : public LResource
{
public:
    RGestureSwipe(Protocols::Wayland::RPointer *rPointer, Int32 id, UInt32 version);
    ~RGestureSwipe();

    struct SerialEvents
    {
        LPointerSwipeBeginEvent begin;
        LPointerSwipeEndEvent end;
    };

    Protocols::Wayland::RPointer *pointerResource() const;
    const SerialEvents &serialEvents() const;

    // Since 1
    bool begin(const LPointerSwipeBeginEvent &event, Protocols::Wayland::RSurface *rSurface);
    bool update(const LPointerSwipeUpdateEvent &event);
    bool end(const LPointerSwipeEndEvent &event);

    LPRIVATE_IMP_UNIQUE(RGestureSwipe);
};

#endif // RGESTURESWIPE_H
