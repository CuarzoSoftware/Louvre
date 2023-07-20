#ifndef LDNDICONROLEPRIVATE_H
#define LDNDICONROLEPRIVATE_H

#include <LDNDIconRole.h>

using namespace Louvre;

struct LDNDIconRole::Params
{
    LSurface *surface;
};

LPRIVATE_CLASS(LDNDIconRole)
    LPoint currentHotspot, pendingHotspotOffset;
    LPoint currentHotspotB;
};

#endif // LDNDICONROLEPRIVATE_H
