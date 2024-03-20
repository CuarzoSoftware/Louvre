#ifndef LSUBSURFACEROLEPRIVATE_H
#define LSUBSURFACEROLEPRIVATE_H

#include <LSubsurfaceRole.h>

using namespace Louvre;

struct LSubsurfaceRole::Params
{
    LResource *subsurface;
    LSurface *surface;
};

LPRIVATE_CLASS(LSubsurfaceRole)
    bool isSynced                                                       = true;
    bool hasCache = true;

    // Local pos
    LPoint currentLocalPos, pendingLocalPos;
    bool hasPendingLocalPos                                             = true;

    // Pending reordering
    LWeak<LSurface> pendingPlaceAbove;
    LWeak<LSurface> pendingPlaceBelow;
};

#endif // LSUBSURFACEROLEPRIVATE_H
