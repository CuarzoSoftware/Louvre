#ifndef LLAYERROLEPRIVATE_H
#define LLAYERROLEPRIVATE_H

#include <CZ/Louvre/Roles/LLayerRole.h>

using namespace Louvre;

struct LLayerRole::Params
{
    LResource *layerSurfaceRes;
    LSurface *surface;
    LOutput *output;
    LSurfaceLayer layer;
    const char *scope;
};

#endif // LLAYERROLEPRIVATE_H
