#ifndef LXWINDOWROLE_H
#define LXWINDOWROLE_H

#include <LBaseSurfaceRole.h>
#include <memory>

class Louvre::LXWindowRole : public LBaseSurfaceRole
{
public:
    struct Params;
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LXWindowRole;

    LXWindowRole(const void *params) noexcept;
    LCLASS_NO_COPY(LXWindowRole)
    ~LXWindowRole();

    LPRIVATE_IMP_UNIQUE(LXWindowRole)
};

#endif // LXWINDOWROLE_H
