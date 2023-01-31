#ifndef LDNDICONROLEPRIVATE_H
#define LDNDICONROLEPRIVATE_H

#include <LDNDIconRole.h>

struct Louvre::LDNDIconRole::Params
{
    LSurface *surface;
};

class Louvre::LDNDIconRole::LDNDIconRolePrivate
{
public:
    LDNDIconRolePrivate()                                       = default;
    ~LDNDIconRolePrivate()                                      = default;

    LDNDIconRolePrivate(const LDNDIconRolePrivate&)             = delete;
    LDNDIconRolePrivate &operator=(const LDNDIconRolePrivate&)  = delete;

    LPoint currentHotspotS, pendingHotspotOffsetS;
    LPoint currentHotspotC;
    LPoint currentHotspotB;

};

#endif // LDNDICONROLEPRIVATE_H
