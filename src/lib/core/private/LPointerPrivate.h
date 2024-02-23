#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

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
        LastCursorRequestWasHide = 1 << 0,
        NaturalScrollX           = 1 << 1,
        NaturalScrollY           = 1 << 2,
        PendingSwipeEndEvent     = 1 << 3,
        PendingPinchEndEvent     = 1 << 4,
        PendingHoldEndEvent      = 1 << 5
    };

    LBitset<StateFlags> state { NaturalScrollX | NaturalScrollY };

    void sendLeaveEvent(LSurface *surface);

    std::weak_ptr<LSurface> focus;
    std::weak_ptr<LSurface> draggingSurface;
    std::vector<Button> pressedButtons;
    Float32 axisXprev;
    Float32 axisYprev;
    LCursorRole *lastCursorRequest = nullptr;
};

#endif // LPOINTERPRIVATE_H
