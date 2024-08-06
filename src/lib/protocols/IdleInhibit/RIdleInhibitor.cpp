#include <protocols/IdleInhibit/idle-inhibit-unstable-v1.h>
#include <protocols/IdleInhibit/RIdleInhibitor.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::IdleInhibit;

static const struct zwp_idle_inhibitor_v1_interface imp
{
    .destroy = &RIdleInhibitor::destroy,
};

RIdleInhibitor::RIdleInhibitor
    (
        LSurface *surface,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        surface->client(),
        &zwp_idle_inhibitor_v1_interface,
        version,
        id,
        &imp
    ),
    m_surface(surface)
{
    m_surface.setOnDestroyCallback([this](LSurface *surface) {
        handleRemoval(this, surface);
    });

    surface->imp()->idleInhibitorResources.emplace_back(this);

    // Prevent duplicates in LSeat::idleInhibitors()
    if (surface->imp()->idleInhibitorResources.size() == 1)
        seat()->imp()->idleInhibitors.emplace_back(surface);

    // TODO: Notify ?
}

RIdleInhibitor::~RIdleInhibitor()
{
    if (m_surface)
        handleRemoval(this, m_surface.get());
}

void RIdleInhibitor::handleRemoval(RIdleInhibitor *idleInhibitorRes, LSurface *surface) noexcept
{
    LVectorRemoveOneUnordered(surface->imp()->idleInhibitorResources, idleInhibitorRes);

    if (surface->imp()->idleInhibitorResources.empty())
        LVectorRemoveOneUnordered(seat()->imp()->idleInhibitors, surface);
}

/******************** REQUESTS ********************/

void RIdleInhibitor::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

