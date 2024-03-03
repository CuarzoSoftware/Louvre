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
        NaturalScrollX           = 1 << 0,
        NaturalScrollY           = 1 << 1,
        PendingSwipeEndEvent     = 1 << 2,
        PendingPinchEndEvent     = 1 << 3,
        PendingHoldEndEvent      = 1 << 4
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
