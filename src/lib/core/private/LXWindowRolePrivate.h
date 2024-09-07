#ifndef LXWINDOWROLEPRIVATE_H
#define LXWINDOWROLEPRIVATE_H

#include <LXWindowRole.h>

using namespace Louvre;

struct LXWindowRole::Params
{
    LResource *xWaylandSurfaceRes;
    LSurface *surface;
};

LPRIVATE_CLASS(LXWindowRole)

};

#endif // LXWINDOWROLEPRIVATE_H
