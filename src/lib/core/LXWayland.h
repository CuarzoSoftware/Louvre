#ifndef LXWAYLAND_H
#define LXWAYLAND_H

#include <LFactoryObject.h>
#include <unordered_map>
#include <memory>

class Louvre::LXWayland : public LFactoryObject
{
public:

    enum State
    {
        Uninitialized,
        Initializing,
        Initialized
    };

    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LXWayland;

    LXWayland(const void *params) noexcept;
    ~LXWayland();

    const std::unordered_map<UInt32, LXWindowRole*> windows() const noexcept;
    State state() const noexcept;
    const std::string &display() const noexcept;

    virtual void onStart();
    virtual void onFail() {};
    virtual void onStop() {};

    void start();

    LPRIVATE_IMP_UNIQUE(LXWayland)
};

#endif // LXWAYLAND_H
