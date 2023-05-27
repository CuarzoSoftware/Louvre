#ifndef LPOPUPROLEPRIVATE_H
#define LPOPUPROLEPRIVATE_H

#include "LPositioner.h"
#include <LPopupRole.h>

struct Louvre::LPopupRole::Params
{
    LResource *popup;
    LSurface *surface;
    LPositioner *positioner;
};

class Louvre::LPopupRole::LPopupRolePrivate
{
public:
    LPopupRolePrivate()                                     = default;
    ~LPopupRolePrivate()                                    = default;

    LPopupRolePrivate(const LPopupRolePrivate&)             = delete;
    LPopupRolePrivate& operator= (const LPopupRolePrivate&) = delete;

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
