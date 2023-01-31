#ifndef LSUBSURFACEROLEPRIVATE_H
#define LSUBSURFACEROLEPRIVATE_H

#include <LSubsurfaceRole.h>

struct Louvre::LSubsurfaceRole::Params
{
    wl_resource *subsurface;
    LSurface *surface;
};

class Louvre::LSubsurfaceRole::LSubsurfaceRolePrivate
{
public:
    LSubsurfaceRolePrivate()                                            = default;
    ~LSubsurfaceRolePrivate()                                           = default;

    LSubsurfaceRolePrivate(const LSubsurfaceRolePrivate&)               = delete;
    LSubsurfaceRolePrivate& operator= (const LSubsurfaceRolePrivate&)   = delete;

    // Indicates the subsurface mode
    bool isSynced                                                       = true;

    // Local pos
    LPoint currentLocalPosS,pendingLocalPosS;
    LPoint currentLocalPosC;
    bool hasPendingLocalPos                                             = true;

    // Pending reordering
    LSurface *pendingPlaceAbove                                         = nullptr;
    LSurface *pendingPlaceBelow                                         = nullptr;

};

#endif // LSUBSURFACEROLEPRIVATE_H
