#include <private/LXWaylandPrivate.h>
#include <LLog.h>
#include <cassert>
#include <unistd.h>

using namespace Louvre;

LXWayland::LXWayland(const void *params) noexcept :
    LFactoryObject(FactoryObjectType),
    LPRIVATE_INIT_UNIQUE(LXWayland)
{
    assert(params != nullptr && "Invalid parameter passed to LXWayland constructor.");
    LXWayland**ptr { (LXWayland**) params };
    assert(*ptr == nullptr && *ptr == xWayland() && "Only a single LXWayland instance can exist.");
    *ptr = this;
    imp()->x = this;
}

LXWayland::~LXWayland()
{

}

const std::unordered_map<UInt32, LXWindowRole *> LXWayland::windows() const noexcept
{
    return imp()->windows;
}

LXWayland::State LXWayland::state() const noexcept
{
    return imp()->state;
}

const std::string &LXWayland::display() const noexcept
{
    return imp()->displayName;
}

void LXWayland::onStart()
{
    LLog::log("Xwayland running on DISPLAY=%s.", display().c_str());
}

void LXWayland::start()
{
    imp()->init();
}
