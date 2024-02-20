#ifndef RGESTUREPINCH_H
#define RGESTUREPINCH_H

#include <LResource.h>
#include <LPointerPinchBeginEvent.h>
#include <LPointerPinchEndEvent.h>

class Louvre::Protocols::PointerGestures::RGesturePinch : public LResource
{
public:
    RGesturePinch(Protocols::Wayland::RPointer *rPointer, Int32 id, UInt32 version);
    ~RGesturePinch();

    Protocols::Wayland::RPointer *pointerResource() const;

    // Since 1
    bool begin(const LPointerPinchBeginEvent &event, Protocols::Wayland::RSurface *rSurface);
    bool update(const LPointerPinchUpdateEvent &event);
    bool end(const LPointerPinchEndEvent &event);

    LPRIVATE_IMP_UNIQUE(RGesturePinch)
};

#endif // RGESTUREPINCH_H
