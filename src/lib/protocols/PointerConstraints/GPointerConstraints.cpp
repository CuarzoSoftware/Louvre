#include <protocols/PointerConstraints/pointer-constraints-unstable-v1.h>
#include <protocols/PointerConstraints/GPointerConstraints.h>
#include <protocols/PointerConstraints/RConfinedPointer.h>
#include <protocols/PointerConstraints/RLockedPointer.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LSurfacePrivate.h>
#include <private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::PointerConstraints;

static const struct zwp_pointer_constraints_v1_interface imp
{
    .destroy = &GPointerConstraints::destroy,
    .lock_pointer = &GPointerConstraints::lock_pointer,
    .confine_pointer = &GPointerConstraints::confine_pointer
};

void GPointerConstraints::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GPointerConstraints(client, version, id);
}

Int32 GPointerConstraints::maxVersion() noexcept
{
    return LOUVRE_POINTER_CONSTRAINTS_VERSION;
}

const wl_interface *GPointerConstraints::interface() noexcept
{
    return &zwp_pointer_constraints_v1_interface;
}

GPointerConstraints::GPointerConstraints(
    wl_client *client,
    Int32 version,
    UInt32 id
    ) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->pointerConstraintsGlobals.emplace_back(this);
}

GPointerConstraints::~GPointerConstraints() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->pointerConstraintsGlobals, this);
}

/******************** REQUESTS ********************/

void GPointerConstraints::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GPointerConstraints::lock_pointer(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *pointer, wl_resource *region, UInt32 lifetime) noexcept
{
    auto &surfaceRes { *static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes.surface()->imp()->lockedPointerRes || surfaceRes.surface()->imp()->confinedPointerRes)
    {
        surfaceRes.postError(ZWP_POINTER_CONSTRAINTS_V1_ERROR_ALREADY_CONSTRAINED,
                               "Pointer constraint already requested on that surface.");
        return;
    }

    if (lifetime != ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT && lifetime != ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT)
    {
        surfaceRes.postError(0, "Invalid lifetime value.");
        return;
    }

    new RLockedPointer(static_cast<GPointerConstraints*>(wl_resource_get_user_data(resource)),
                       surfaceRes.surface(),
                       static_cast<Wayland::RPointer*>(wl_resource_get_user_data(pointer)),
                       region == NULL ? nullptr : static_cast<Wayland::RRegion*>(wl_resource_get_user_data(region)),
                       lifetime,
                       id);
}

void GPointerConstraints::confine_pointer(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *pointer, wl_resource *region, UInt32 lifetime) noexcept
{
    auto &surfaceRes { *static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes.surface()->imp()->lockedPointerRes || surfaceRes.surface()->imp()->confinedPointerRes)
    {
        surfaceRes.postError(ZWP_POINTER_CONSTRAINTS_V1_ERROR_ALREADY_CONSTRAINED,
                               "Pointer constraint already requested on that surface.");
        return;
    }

    if (lifetime != ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT && lifetime != ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT)
    {
        surfaceRes.postError(0, "Invalid lifetime value.");
        return;
    }

    new RConfinedPointer(static_cast<GPointerConstraints*>(wl_resource_get_user_data(resource)),
                       surfaceRes.surface(),
                       static_cast<Wayland::RPointer*>(wl_resource_get_user_data(pointer)),
                       region == NULL ? nullptr : static_cast<Wayland::RRegion*>(wl_resource_get_user_data(region)),
                       lifetime,
                       id);
}
