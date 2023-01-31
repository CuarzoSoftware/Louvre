#ifndef LBASESURFACEROLEPRIVATE_H
#define LBASESURFACEROLEPRIVATE_H

#include <LBaseSurfaceRole.h>

class Louvre::LBaseSurfaceRole::LBaseSurfaceRolePrivate
{
public:
    LBaseSurfaceRolePrivate()                                           = default;
    ~LBaseSurfaceRolePrivate()                                          = default;

    LBaseSurfaceRolePrivate(const LBaseSurfaceRolePrivate&)             = delete;
    LBaseSurfaceRolePrivate &operator=(const LBaseSurfaceRolePrivate&)  = delete;

    wl_resource *resource                                               = nullptr;
    LSurface *surface                                                   = nullptr;
    LCompositor *compositor                                             = nullptr;
    UInt32 roleId;

};

#endif // LBASESURFACEROLEPRIVATE_H
