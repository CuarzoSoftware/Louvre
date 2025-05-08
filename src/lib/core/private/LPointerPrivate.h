#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

#include <LSurface.h>
#include <LPointer.h>
#include <LBitset.h>

using namespace Louvre;

LPRIVATE_CLASS(LPointer)

    enum StateFlags
    {
        NaturalScrollX           = static_cast<UInt32>(1) << 0,
        NaturalScrollY           = static_cast<UInt32>(1) << 1,
        PendingSwipeEndEvent     = static_cast<UInt32>(1) << 2,
        PendingPinchEndEvent     = static_cast<UInt32>(1) << 3,
        PendingHoldEndEvent      = static_cast<UInt32>(1) << 4
    };

    void sendLeaveEvent(LSurface *surface) noexcept;

    LWeak<LSurface> focus;
    LWeak<LSurface> draggingSurface;
    std::vector<LPointerButtonEvent::Button> pressedButtons;
    LBitset<StateFlags> state { NaturalScrollX | NaturalScrollY };
};

#endif // LPOINTERPRIVATE_H
