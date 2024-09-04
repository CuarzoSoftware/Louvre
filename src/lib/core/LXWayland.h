#ifndef LXWAYLAND_H
#define LXWAYLAND_H

#include <LFactoryObject.h>
#include <memory>

class Louvre::LXWayland : public LFactoryObject
{
public:
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LXWayland;

    LXWayland(const void *params) noexcept;
    ~LXWayland();

    bool start();
    void started();

    LPRIVATE_IMP_UNIQUE(LXWayland)
};

#endif // LXWAYLAND_H
