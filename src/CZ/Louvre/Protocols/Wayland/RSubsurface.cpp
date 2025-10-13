#include <CZ/Louvre/Protocols/Wayland/RSubsurface.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/GSubcompositor.h>
#include <CZ/Louvre/Private/LSubsurfaceRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::Wayland;

static const struct wl_subsurface_interface imp
{
    .destroy = &RSubsurface::destroy,
    .set_position = &RSubsurface::set_position,
    .place_above = &RSubsurface::place_above,
    .place_below = &RSubsurface::place_below,
    .set_sync = &RSubsurface::set_sync,
    .set_desync = &RSubsurface::set_desync
};

RSubsurface::RSubsurface
(
    GSubcompositor *subcompositorRes,
    LSurface *surface,
    LSurface *parent,
    UInt32 id
)
    :LResource
    (
        surface->client(),
        &wl_subsurface_interface,
        subcompositorRes->version(),
        id,
        &imp
    )
{
    LSubsurfaceRole::Params subsurfaceRoleParams { this, surface, parent };
    m_subsurfaceRole.reset(LFactory::createObject<LSubsurfaceRole>(&subsurfaceRoleParams));
    surface->imp()->notifyRoleChange();
    parent->imp()->pending.subsurfacesAbove.emplace_back(m_subsurfaceRole.get());
    surface->imp()->setParent(parent);
}

RSubsurface::~RSubsurface()
{
    auto *surface { subsurfaceRole()->surface() };
    auto *parent { surface->parent() };
    compositor()->onAnticipatedObjectDestruction(m_subsurfaceRole.get());
    surface->imp()->setMapped(false);
    surface->imp()->setRole(nullptr, true);

    if (parent)
    {
        CZVectorUtils::RemoveOne(parent->imp()->current.subsurfacesAbove, m_subsurfaceRole.get());
        CZVectorUtils::RemoveOne(parent->imp()->current.subsurfacesBelow, m_subsurfaceRole.get());
        surface->imp()->setParent(parent);
    }

    m_subsurfaceRole.reset();
}

/******************** REQUESTS ********************/

void RSubsurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RSubsurface::set_position(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y)
{
    auto &subsurface { *static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };
    subsurface.m_pendingLocalPos.fX = x;
    subsurface.m_pendingLocalPos.fY = y;
}

static bool InsertAbove(std::vector<CZWeak<LSubsurfaceRole>> &vec, LSubsurfaceRole *sibling, CZWeak<LSubsurfaceRole> subsurface) noexcept
{
    const auto it { std::find(vec.begin(), vec.end(), sibling) };

    if (it == vec.end()) return false;

    if (it == std::prev(vec.end()))
        vec.emplace_back(subsurface);
    else
        vec.insert(std::next(it), subsurface);

    return true;
}

void RSubsurface::place_above(wl_client */*client*/, wl_resource *resource, wl_resource *sibling)
{
    auto &res { *static_cast<RSubsurface*>(wl_resource_get_user_data(resource)) };
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };
    LSurface *siblingSurface { static_cast<RWlSurface*>(wl_resource_get_user_data(sibling))->surface() };
    auto *parent { subsurfaceRole->surface()->parent() };

    assert(parent);

    CZWeak<LSubsurfaceRole> weakRef { subsurfaceRole };
    CZVectorUtils::RemoveOne(parent->imp()->pending.subsurfacesAbove, weakRef);
    CZVectorUtils::RemoveOne(parent->imp()->pending.subsurfacesBelow, weakRef);

    if (parent == siblingSurface)
    {
        parent->imp()->pending.subsurfacesAbove.insert(parent->imp()->pending.subsurfacesAbove.begin(), weakRef);
        return;
    }

    if (auto *siblingSubsurface = siblingSurface->subsurface())
    {
        if (InsertAbove(parent->imp()->pending.subsurfacesAbove, siblingSubsurface, weakRef))
            return;

        if (InsertAbove(parent->imp()->pending.subsurfacesBelow, siblingSubsurface, weakRef))
            return;
    }

    res.postError(WL_SUBSURFACE_ERROR_BAD_SURFACE, "Subsurface is not sibling or parent");
}

static bool InsertBelow(std::vector<CZWeak<LSubsurfaceRole>> &vec, LSubsurfaceRole *sibling, CZWeak<LSubsurfaceRole> subsurface) noexcept
{
    const auto it { std::find(vec.begin(), vec.end(), sibling) };
    if (it == vec.end()) return false;
    vec.insert(it, subsurface);
    return true;
}

void RSubsurface::place_below(wl_client */*client*/, wl_resource *resource, wl_resource *sibling)
{
    auto &res { *static_cast<RSubsurface*>(wl_resource_get_user_data(resource)) };
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };
    LSurface *siblingSurface { static_cast<RWlSurface*>(wl_resource_get_user_data(sibling))->surface() };

    auto *parent { subsurfaceRole->surface()->parent() };

    assert(parent);

    CZWeak<LSubsurfaceRole> weakRef { subsurfaceRole };
    CZVectorUtils::RemoveOne(parent->imp()->pending.subsurfacesAbove, weakRef);
    CZVectorUtils::RemoveOne(parent->imp()->pending.subsurfacesBelow, weakRef);

    if (parent == siblingSurface)
    {
        parent->imp()->pending.subsurfacesBelow.emplace_back(weakRef);
        return;
    }

    if (auto *siblingSubsurface = siblingSurface->subsurface())
    {
        if (InsertBelow(parent->imp()->pending.subsurfacesAbove, siblingSubsurface, weakRef))
            return;

        if (InsertBelow(parent->imp()->pending.subsurfacesBelow, siblingSubsurface, weakRef))
            return;
    }

    res.postError(WL_SUBSURFACE_ERROR_BAD_SURFACE, "Subsurface is not sibling or parent");
}

static bool HasSyncParent(LSubsurfaceRole *subsurface) noexcept
{
    if (!subsurface || !subsurface->surface()->parent() || !subsurface->surface()->parent()->subsurface())
        return false;

    if (subsurface->surface()->parent()->subsurface()->isSynced())
        return true;

    return HasSyncParent(subsurface->surface()->parent()->subsurface());
}

static void SyncSubsurfaces(LSurface *surface) noexcept
{
    for (LSubsurfaceRole *s : surface->subsurfacesAbove())
        RSubsurface::set_sync(nullptr, s->resource()->resource());

    for (LSubsurfaceRole *s : surface->subsurfacesBelow())
        RSubsurface::set_sync(nullptr, s->resource()->resource());
}

void RSubsurface::set_sync(wl_client */*client*/, wl_resource *resource)
{
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };

    if (!subsurfaceRole->isSynced())
    {
        subsurfaceRole->m_isSynced = true;
        subsurfaceRole->syncModeChanged();
        SyncSubsurfaces(subsurfaceRole->surface());
    }
}

void RSubsurface::set_desync(wl_client */*client*/, wl_resource *resource)
{
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };

    if (subsurfaceRole->isSynced() && !HasSyncParent(subsurfaceRole))
    {
        subsurfaceRole->m_isSynced = false;
        subsurfaceRole->syncModeChanged();
        subsurfaceRole->m_lock.reset();
    }
}
