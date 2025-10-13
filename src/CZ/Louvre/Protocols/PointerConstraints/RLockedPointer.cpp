#include <CZ/Louvre/Protocols/PointerConstraints/pointer-constraints-unstable-v1.h>
#include <CZ/Louvre/Protocols/PointerConstraints/GPointerConstraints.h>
#include <CZ/Louvre/Protocols/PointerConstraints/RLockedPointer.h>
#include <CZ/Louvre/Protocols/Wayland/RPointer.h>
#include <CZ/Louvre/Protocols/Wayland/RRegion.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>

using namespace CZ::Protocols::PointerConstraints;

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
    auto &imp { *m_surface->imp() };
    imp.pending.lockedPointerPosHint = { -1.f, -1.f };
    imp.pending.lockedPointerRes.reset(this);

    if (regionRes)
        imp.pending.pointerConstraintRegion = std::make_unique<SkRegion>(regionRes->region());
    else
        imp.pending.pointerConstraintRegion.reset();

    imp.pending.changesToNotify.add(LSurfaceCommitEvent::PointerConstraintRegionChanged | LSurfaceCommitEvent::LockedPointerPosHintChanged);
}

RLockedPointer::~RLockedPointer()
{
    if (surface())
    {
        auto &imp { *surface()->imp() };
        imp.pending.pointerConstraintRegion.reset();
        imp.pending.lockedPointerPosHint = SkPoint::Make(-1.f, -1.f);
        imp.pending.changesToNotify.add(LSurfaceCommitEvent::PointerConstraintRegionChanged | LSurfaceCommitEvent::LockedPointerPosHintChanged);
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
        auto &imp { *res.surface()->imp() };
        imp.pending.lockedPointerPosHint.fX = wl_fixed_to_double(x);
        imp.pending.lockedPointerPosHint.fY = wl_fixed_to_double(y);
        imp.pending.changesToNotify.add(LSurfaceCommitEvent::LockedPointerPosHintChanged);
    }
}

void RLockedPointer::set_region(wl_client */*client*/, wl_resource *resource, wl_resource *region) noexcept
{
    auto &res { *static_cast<RLockedPointer*>(wl_resource_get_user_data(resource)) };

    if (res.surface())
    {
        auto &imp { *res.surface()->imp() };

        if (region)
        {
            auto *regionRes { static_cast<Wayland::RRegion*>(wl_resource_get_user_data(region)) };
            imp.pending.pointerConstraintRegion = std::make_unique<SkRegion>(regionRes->region());
        }
        else
            imp.pending.pointerConstraintRegion.reset();

        imp.pending.changesToNotify.add(LSurfaceCommitEvent::LockedPointerPosHintChanged);
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
            auto &imp { *surface()->imp() };
            imp.pending.lockedPointerRes.reset();
            imp.pending.pointerConstraintRegion.reset();
            imp.pending.lockedPointerPosHint = { -1.f, -1.f };
            imp.pending.changesToNotify.remove(
                LSurfaceCommitEvent::PointerConstraintRegionChanged |
                LSurfaceCommitEvent::LockedPointerPosHintChanged);
            m_surface.reset();
        }
    }
}
