#include <CZ/Louvre/Protocols/PointerConstraints/pointer-constraints-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerConstraints/GPointerConstraints.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RConfinedPointer.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RLockedPointer.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PointerConstraints;

static const struct zwp_pointer_constraints_v1_interface imp
{
    .destroy = &GPointerConstraints::destroy,
    .lock_pointer = &GPointerConstraints::lock_pointer,
    .confine_pointer = &GPointerConstraints::confine_pointer
};

LGLOBAL_INTERFACE_IMP(GPointerConstraints, LOUVRE_POINTER_CONSTRAINTS_VERSION, zwp_pointer_constraints_v1_interface)

bool GPointerConstraints::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.PointerConstraints)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.PointerConstraints;
    return true;
}

GPointerConstraints::GPointerConstraints(
    wl_client *client,
    Int32 version,
    UInt32 id
    )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->pointerConstraintsGlobals.emplace_back(this);
}

GPointerConstraints::~GPointerConstraints() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->pointerConstraintsGlobals, this);
}

/******************** REQUESTS ********************/

void GPointerConstraints::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GPointerConstraints::lock_pointer(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface, wl_resource *pointer, wl_resource *region, UInt32 lifetime) noexcept
{
    auto &surfaceRes { *static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes.surface()->imp()->pending.lockedPointerRes || surfaceRes.surface()->imp()->pending.confinedPointerRes)
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
    auto &surfaceRes { *static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes.surface()->imp()->pending.lockedPointerRes || surfaceRes.surface()->imp()->pending.confinedPointerRes)
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
