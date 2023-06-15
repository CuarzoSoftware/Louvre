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
    LRect positionerBoundsS, positionerBoundsC;
    LPositioner positioner;
    bool dismissed = false;

    // Since 3
    UInt32 repositionSerial = 0;
};

#endif // LPOPUPROLEPRIVATE_H
