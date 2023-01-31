#ifndef LCURSORROLEPRIVATE_H
#define LCURSORROLEPRIVATE_H

#include <LCursorRole.h>

struct Louvre::LCursorRole::Params
{
    LSurface *surface;
};

class Louvre::LCursorRole::LCursorRolePrivate
{
public:
    LCursorRolePrivate()                                        = default;
    ~LCursorRolePrivate()                                       = default;

    LCursorRolePrivate(const LCursorRolePrivate&)               = delete;
    LCursorRolePrivate &operator=(const LCursorRolePrivate&)    = delete;

    LPoint currentHotspotS, pendingHotspotOffsetS;
    LPoint currentHotspotC;
    LPoint currentHotspotB;
};

#endif // LCURSORROLEPRIVATE_H
