#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

#include <LSurface.h>
#include <LPointer.h>
#include <LBitset.h>

using namespace Louvre;

struct LPointer::Params
{
    /* Add here any required constructor param */
};

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

    LBitset<StateFlags> state { NaturalScrollX | NaturalScrollY };
    LWeak<LSurface> focus;
    LWeak<LSurface> draggingSurface;
    std::vector<LPointerButtonEvent::Button> pressedButtons;
    Float32 axisXprev;
    Float32 axisYprev;
};

#endif // LPOINTERPRIVATE_H
