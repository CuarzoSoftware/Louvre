#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

#include <LPointer.h>

struct Louvre::LPointer::Params
{
    LSeat *seat;
};

class Louvre::LPointer::LPointerPrivate
{
public:
    LPointerPrivate()                                   = default;
    ~LPointerPrivate()                                  = default;

    LPointerPrivate(const LPointerPrivate&)             = delete;
    LPointerPrivate& operator= (const LPointerPrivate&) = delete;

    // Events
    void sendLeaveEvent(LSurface *surface);
    void sendEnterEvent(LSurface *surface, const LPoint &point);

    // Wayland
    LSeat *seat                                         = nullptr;
    LSurface *pointerFocusSurface                       = nullptr;
    LSurface *draggingSurface                           = nullptr;
    //LSurface *cursorSurface                             = nullptr;
    LToplevelRole *movingToplevel                       = nullptr;
    LToplevelRole *resizingToplevel                     = nullptr;

    // Toplevel Moving
    LPoint movingToplevelInitPos;
    LPoint movingToplevelInitCursorPos;
    LRect movingToplevelConstraintBounds;

    // Resizing
    LPoint resizingToplevelInitPos;
    LPoint resizingToplevelInitCursorPos;
    //LSize resizingToplevelInitSize;
    LSize resizingToplevelInitWindowSize;
    LToplevelRole::ResizeEdge resizingToplevelEdge;
    LRect resizingToplevelConstraintBounds;

    UInt32 lastPointerEnterEventSerial                  = 0;
    UInt32 lastPointerLeaveEventSerial                  = 0;
    UInt32 lastPointerButtonEventSerial                 = 0;

    // Axis
#if LOUVRE_SEAT_VERSION >= 5
    LPoint axisDiscreteStep                             = LPoint(15,15);
#endif

};


#endif // LPOINTERPRIVATE_H
