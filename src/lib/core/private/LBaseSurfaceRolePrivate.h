#ifndef LBASESURFACEROLEPRIVATE_H
#define LBASESURFACEROLEPRIVATE_H

#include <LBaseSurfaceRole.h>

using namespace Louvre;

LPRIVATE_CLASS(LBaseSurfaceRole)
    LCompositor *compositor;
    LResource *resource;
    LSurface *surface;
    UInt32 roleId;
};

#endif // LBASESURFACEROLEPRIVATE_H
