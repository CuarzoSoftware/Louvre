#ifndef LSUBSURFACEROLEPRIVATE_H
#define LSUBSURFACEROLEPRIVATE_H

#include <LSubsurfaceRole.h>

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
    LSurface *pendingPlaceAbove                                         = nullptr;
    wl_listener pendingPlaceAboveDestroyListener;
    LSurface *pendingPlaceBelow                                         = nullptr;
    wl_listener pendingPlaceBelowDestroyListener;
};

#endif // LSUBSURFACEROLEPRIVATE_H
