#ifndef LPOPUPROLEPRIVATE_H
#define LPOPUPROLEPRIVATE_H

#include <CZ/Louvre/Roles/LPopupRole.h>

using namespace CZ;

struct LPopupRole::Params
{
    LResource *popup;
    LSurface *surface;
    LPositioner *positioner;
};

#endif // LPOPUPROLEPRIVATE_H
