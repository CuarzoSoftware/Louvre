#ifndef LSUBSURFACEROLEPRIVATE_H
#define LSUBSURFACEROLEPRIVATE_H

#include <CZ/Louvre/Roles/LSubsurfaceRole.h>

using namespace Louvre;

struct LSubsurfaceRole::Params
{
    LResource *subsurface;
    LSurface *surface;
};

#endif // LSUBSURFACEROLEPRIVATE_H
