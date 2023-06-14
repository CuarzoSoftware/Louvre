#ifndef LSUBSURFACEROLEPRIVATE_H
#define LSUBSURFACEROLEPRIVATE_H

#include <LSubsurfaceRole.h>

struct LSubsurfaceRole::Params
{
    LResource *subsurface;
    LSurface *surface;
};

LPRIVATE_CLASS(LSubsurfaceRole)

    // Indicates the subsurface mode
    bool isSynced                                                       = true;

    // Local pos
    LPoint currentLocalPosS,pendingLocalPosS;
    LPoint currentLocalPosC;
    bool hasPendingLocalPos                                             = true;

    // Pending reordering
    LSurface *pendingPlaceAbove                                         = nullptr;
    wl_listener pendingPlaceAboveDestroyListener;
    LSurface *pendingPlaceBelow                                         = nullptr;
    wl_listener pendingPlaceBelowDestroyListener;
};

#endif // LSUBSURFACEROLEPRIVATE_H
