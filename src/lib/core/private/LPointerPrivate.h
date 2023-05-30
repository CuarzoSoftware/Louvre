#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

#include <LPointer.h>

using namespace Louvre;

struct LPointer::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LPointer)
    // Events
    void sendLeaveEvent(LSurface *surface);
    void sendEnterEvent(LSurface *surface, const LPoint &point);

    // Wayland
    LSurface *pointerFocusSurface = nullptr;
    LSurface *draggingSurface = nullptr;
    LToplevelRole *movingToplevel = nullptr;
    LToplevelRole *resizingToplevel = nullptr;

    // Toplevel Moving
    LPoint movingToplevelInitPos;
    LPoint movingToplevelInitCursorPos;
    LRect movingToplevelConstraintBounds;

    // Resizing
    LPoint resizingToplevelInitPos;
    LPoint resizingToplevelInitCursorPos;
    LSize resizingToplevelInitWindowSize;
    LToplevelRole::ResizeEdge resizingToplevelEdge;
    LRect resizingToplevelConstraintBounds;

    // Axis
#if LOUVRE_SEAT_VERSION >= 5
    LPoint axisDiscreteStep = LPoint(15,15);
#endif
};

#endif // LPOINTERPRIVATE_H
