#ifndef LCURSORROLEPRIVATE_H
#define LCURSORROLEPRIVATE_H

#include <LCursorRole.h>

using namespace Louvre;

struct LCursorRole::Params
{
    LSurface *surface;
};

LPRIVATE_CLASS(LCursorRole)
    LPoint currentHotspotS, pendingHotspotOffsetS;
    LPoint currentHotspotC;
    LPoint currentHotspotB;
};

#endif // LCURSORROLEPRIVATE_H
