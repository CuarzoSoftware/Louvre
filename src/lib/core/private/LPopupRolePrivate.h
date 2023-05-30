#ifndef LPOPUPROLEPRIVATE_H
#define LPOPUPROLEPRIVATE_H

#include <LPositioner.h>
#include <LPopupRole.h>

struct LPopupRole::Params
{
    LResource *popup;
    LSurface *surface;
    LPositioner *positioner;
};

LPRIVATE_CLASS(LPopupRole)
    bool hasPendingWindowGeometry                           = false;
    bool windowGeometrySet                                  = false;
    LRect currentWindowGeometryS, pendingWindowGeometryS;
    LRect currentWindowGeometryC;
    LRect positionerBoundsS, positionerBoundsC;

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    UInt32 repositionSerial                                 = 0;
#endif

    LPositioner positioner;
    bool dismissed                                          = false;
};

#endif // LPOPUPROLEPRIVATE_H
