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
    LRect positionerBounds;
    LPositioner positioner;
    bool dismissed = false;
};

#endif // LPOPUPROLEPRIVATE_H
