#include <protocols/PointerConstraints/pointer-constraints-unstable-v1.h>
#include <protocols/PointerConstraints/GPointerConstraints.h>
#include <protocols/PointerConstraints/RLockedPointer.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/RRegion.h>
#include <private/LSurfacePrivate.h>

using namespace Louvre::Protocols::PointerConstraints;

static const struct zwp_locked_pointer_v1_interface imp
{
    .destroy = RLockedPointer::destroy,
    .set_cursor_position_hint = RLockedPointer::set_cursor_position_hint,
    .set_region = RLockedPointer::set_region
};

RLockedPointer::RLockedPointer(GPointerConstraints *pointerConstraintsRes,
    LSurface *surface,
    Wayland::RPointer *pointerRes,
    Wayland::RRegion *regionRes,
    UInt32 lifetime,
    UInt32 id) noexcept
    :LResource
    (
        pointerConstraintsRes->client(),
        &zwp_locked_pointer_v1_interface,
        pointerConstraintsRes->version(),
        id,
        &imp
    ),
    m_pointerRes(pointerRes),
    m_surface(surface),
    m_lifetime(lifetime)
{
    surface->imp()->lockedPointerRes.reset(this);

    if (regionRes)
    {
        m_surface.get()->imp()->pendingPointerConstraintRegion = std::make_unique<LRegion>(regionRes->region());
        m_surface.get()->imp()->pointerConstraintRegion = regionRes->region();
        m_surface.get()->imp()->pointerConstraintRegion.intersectRegion(
            m_surface.get()->imp()->currentInputRegion);
    }
    else
    {
        m_surface.get()->imp()->pointerConstraintRegion = m_surface.get()->imp()->currentInputRegion;
    }

    surface->pointerConstraintModeChanged();
}

RLockedPointer::~RLockedPointer()
{
    if (surface())
    {
        surface()->imp()->lockedPointerRes.reset();
        surface()->imp()->pointerConstraintRegion.clear();
        surface()->imp()->pendingPointerConstraintRegion.reset();
        surface()->imp()->pending.lockedPointerPosHint = LPoint(-1, -1);
        surface()->imp()->current.lockedPointerPosHint = LPoint(-1, -1);
        surface()->imp()->changesToNotify.remove(LSurface::LSurfacePrivate::PointerConstraintRegionChanged |
                                                 LSurface::LSurfacePrivate::LockedPointerPosHintChanged);
        surface()->pointerConstraintModeChanged();
    }
}

void RLockedPointer::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RLockedPointer::set_cursor_position_hint(wl_client */*client*/, wl_resource *resource, Float24 x, Float24 y) noexcept
{
    auto &res { *static_cast<RLockedPointer*>(wl_resource_get_user_data(resource)) };

    if (res.surface())
    {
        res.surface()->imp()->pending.lockedPointerPosHint.setX(wl_fixed_to_double(x));
        res.surface()->imp()->pending.lockedPointerPosHint.setY(wl_fixed_to_double(y));
        res.surface()->imp()->changesToNotify.add(LSurface::LSurfacePrivate::LockedPointerPosHintChanged);
    }
}

void RLockedPointer::set_region(wl_client */*client*/, wl_resource *resource, wl_resource *region) noexcept
{
    auto &res { *static_cast<RLockedPointer*>(wl_resource_get_user_data(resource)) };

    if (res.surface())
    {
        if (region)
        {
            auto *regionRes { static_cast<Wayland::RRegion*>(wl_resource_get_user_data(region)) };
            res.surface()->imp()->pendingPointerConstraintRegion = std::make_unique<LRegion>(regionRes->region());
        }
        else
            res.surface()->imp()->pendingPointerConstraintRegion.reset();

        res.surface()->imp()->changesToNotify.add(LSurface::LSurfacePrivate::LockedPointerPosHintChanged);
    }
}

void RLockedPointer::locked() noexcept
{
    if (m_constrained)
        return;

    m_constrained = true;
    zwp_locked_pointer_v1_send_locked(resource());
}

void RLockedPointer::unlocked()
{
    if (!m_constrained)
        return;

    m_constrained = false;
    zwp_locked_pointer_v1_send_unlocked(resource());

    if (lifetime() == ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_ONESHOT)
    {
        if (surface())
        {
            surface()->imp()->lockedPointerRes.reset();
            surface()->imp()->pointerConstraintRegion.clear();
            surface()->imp()->pendingPointerConstraintRegion.reset();
            surface()->imp()->pending.lockedPointerPosHint = LPoint(-1, -1);
            surface()->imp()->current.lockedPointerPosHint = LPoint(-1, -1);
            surface()->imp()->changesToNotify.remove(LSurface::LSurfacePrivate::PointerConstraintRegionChanged |
                                                     LSurface::LSurfacePrivate::LockedPointerPosHintChanged);
            surface()->pointerConstraintModeChanged();
            m_surface.reset();
        }
    }
}
