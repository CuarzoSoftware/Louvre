#ifndef LDNDICONROLEPRIVATE_H
#define LDNDICONROLEPRIVATE_H

#include <LDNDIconRole.h>

using namespace Louvre;

struct LDNDIconRole::Params
{
    LSurface *surface;
};

LPRIVATE_CLASS(LDNDIconRole)
    LPoint currentHotspotS, pendingHotspotOffsetS;
    LPoint currentHotspotC;
    LPoint currentHotspotB;
};

#endif // LDNDICONROLEPRIVATE_H
