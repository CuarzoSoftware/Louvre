#ifndef LSUBSURFACEROLEPRIVATE_H
#define LSUBSURFACEROLEPRIVATE_H

#include <CZ/Louvre/Roles/LSubsurfaceRole.h>

using namespace CZ;

struct LSubsurfaceRole::Params
{
    Protocols::Wayland::RSubsurface *subsurface;
    LSurface *surface;
    LSurface *parent;
};

#endif // LSUBSURFACEROLEPRIVATE_H
