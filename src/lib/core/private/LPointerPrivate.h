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
        NaturalScrollY           = 1 << 2
    };

    LBitset<StateFlags> state { NaturalScrollX | NaturalScrollY };

    void sendLeaveEvent(LSurface *surface);

    LSurface *pointerFocusSurface = nullptr;
    LSurface *draggingSurface = nullptr;
    LToplevelRole *movingToplevel = nullptr;
    LToplevelRole *resizingToplevel = nullptr;

    LPoint movingToplevelInitPos;
    LPoint movingToplevelInitPointerPos;
    LRect movingToplevelConstraintBounds;

    std::vector<Button> pressedButtons;

    Float32 axisXprev;
    Float32 axisYprev;

    LCursorRole *lastCursorRequest = nullptr;
};

#endif // LPOINTERPRIVATE_H
