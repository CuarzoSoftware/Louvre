#include <CZ/Louvre/Protocols/PointerConstraints/pointer-constraints-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerConstraints/GPointerConstraints.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RConfinedPointer.h>
#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Louvre/Protocols/Wayland/RRegion.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>

using namespace Louvre::Protocols::PointerConstraints;

static const struct zwp_confined_pointer_v1_interface imp
{
    .destroy = RConfinedPointer::destroy,
    .set_region = RConfinedPointer::set_region
};

RConfinedPointer::RConfinedPointer(
    GPointerConstraints *pointerConstraintsRes,
    LSurface *surface,
    Wayland::RPointer *pointerRes,
    Wayland::RRegion *regionRes,
    UInt32 lifetime,
    UInt32 id) noexcept
    :LResource
    (
        pointerConstraintsRes->client(),
        &zwp_confined_pointer_v1_interface,
        pointerConstraintsRes->version(),
        id,
        &imp
    ),
    m_pointerRes(pointerRes),
    m_surface(surface),
    m_lifetime(lifetime)
{
    surface->imp()->confinedPointerRes.reset(this);

    if (regionRes)
    {
        m_surface->imp()->pendingPointerConstraintRegion = std::make_unique<SkRegion>(regionRes->region());
        m_surface->imp()->pointerConstraintRegion.op(
            regionRes->region(),
            m_surface->imp()->currentInputRegion,
            SkRegion::kIntersect_Op);
    }
    else
    {
        m_surface->imp()->pointerConstraintRegion = m_surface->imp()->currentInputRegion;
    }

    surface->pointerConstraintModeChanged();
}

RConfinedPointer::~RConfinedPointer()
{
    if (surface())
    {
        surface()->imp()->confinedPointerRes.reset();
        surface()->imp()->pointerConstraintRegion.setEmpty();
        surface()->imp()->pendingPointerConstraintRegion.reset();
        surface()->imp()->changesToNotify.remove(LSurface::LSurfacePrivate::PointerConstraintRegionChanged);
        surface()->pointerConstraintModeChanged();
    }
}

/******************** REQUESTS ********************/

void RConfinedPointer::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RConfinedPointer::set_region(wl_client */*client*/, wl_resource *resource, wl_resource *region) noexcept
{
    auto &res { *static_cast<RConfinedPointer*>(wl_resource_get_user_data(resource)) };

    if (res.surface())
    {
        if (region)
        {
            auto *regionRes { static_cast<Wayland::RRegion*>(wl_resource_get_user_data(region)) };
            res.surface()->imp()->pendingPointerConstraintRegion = std::make_unique<SkRegion>(regionRes->region());
        }
        else
            res.surface()->imp()->pendingPointerConstraintRegion.reset();

        res.surface()->imp()->changesToNotify.add(LSurface::LSurfacePrivate::LockedPointerPosHintChanged);
    }
}

/******************** EVENTS ********************/

void RConfinedPointer::confined() noexcept
{
    if (m_constrained)
        return;

    m_constrained = true;
    zwp_confined_pointer_v1_send_confined(resource());
}

void RConfinedPointer::unconfined()
{
    if (!m_constrained)
        return;

    m_constrained = false;
    zwp_confined_pointer_v1_send_unconfined(resource());

    if (lifetime() == ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT)
    {
        if (surface())
        {
            surface()->imp()->confinedPointerRes.reset();
            surface()->imp()->pointerConstraintRegion.setEmpty();
            surface()->imp()->pendingPointerConstraintRegion.reset();
            surface()->imp()->changesToNotify.remove(LSurface::LSurfacePrivate::PointerConstraintRegionChanged);
            surface()->pointerConstraintModeChanged();
            m_surface.reset();
        }
    }
}
