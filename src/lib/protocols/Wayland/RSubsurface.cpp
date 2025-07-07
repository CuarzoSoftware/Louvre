#include <protocols/Wayland/RSubsurface.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/GSubcompositor.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LFactory.h>
#include <LCompositor.h>

using namespace Louvre::Protocols::Wayland;

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
    LSubsurfaceRole::Params subsurfaceRoleParams { this, surface };
    m_subsurfaceRole.reset(LFactory::createObject<LSubsurfaceRole>(&subsurfaceRoleParams));

    // According to the wl_subsurface spec, the parent should be applied when parent commits
    surface->imp()->setPendingParent(parent);
    surface->imp()->notifyRoleChange();
}

RSubsurface::~RSubsurface()
{
    compositor()->onAnticipatedObjectDestruction(m_subsurfaceRole.get());
    subsurfaceRole()->surface()->imp()->setMapped(false);
    subsurfaceRole()->surface()->imp()->setRole(nullptr);
    subsurfaceRole()->surface()->imp()->notifyRoleChange();
}

/******************** REQUESTS ********************/

void RSubsurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RSubsurface::set_position(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y)
{
    auto &subsurface { *static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };
    subsurface.m_pendingLocalPos.setX(x);
    subsurface.m_pendingLocalPos.setY(y);
    subsurface.m_hasPendingLocalPos = true;
}

void RSubsurface::place_above(wl_client */*client*/, wl_resource *resource, wl_resource *sibling)
{
    auto &res { *static_cast<RSubsurface*>(wl_resource_get_user_data(resource)) };
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };
    LSurface *siblingSurface { static_cast<RSurface*>(wl_resource_get_user_data(sibling))->surface() };

    const bool siblingIsParent { subsurfaceRole->surface()->parent() && subsurfaceRole->surface()->parent() == siblingSurface };

    if (siblingIsParent  || (siblingSurface->parent() == subsurfaceRole->surface()->parent() && siblingSurface != subsurfaceRole->surface()))
    {
        subsurfaceRole->m_pendingPlace.reset(siblingSurface);
        subsurfaceRole->m_pendingPlaceAbove = true;
        return;
    }

    res.postError(WL_SUBSURFACE_ERROR_BAD_SURFACE, "Subsurface is not sibling or parent.");
}

void RSubsurface::place_below(wl_client */*client*/, wl_resource *resource, wl_resource *sibling)
{
    auto &res { *static_cast<RSubsurface*>(wl_resource_get_user_data(resource)) };
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };
    LSurface *siblingSurface { static_cast<RSurface*>(wl_resource_get_user_data(sibling))->surface() };

    const bool siblingIsParent { subsurfaceRole->surface()->parent() && subsurfaceRole->surface()->parent() == siblingSurface };

    if (siblingIsParent  || (siblingSurface->parent() == subsurfaceRole->surface()->parent() && siblingSurface != subsurfaceRole->surface()))
    {
        subsurfaceRole->m_pendingPlace.reset(siblingSurface);
        subsurfaceRole->m_pendingPlaceAbove = false;
        return;
    }

    res.postError(WL_SUBSURFACE_ERROR_BAD_SURFACE, "Subsurface is not sibling.");
}

static bool hasSyncParent(LSurface *surface)
{
    if (surface->parent())
    {
        if (surface->parent()->subsurface())
            return surface->parent()->subsurface()->isSynced();
        else
            return hasSyncParent(surface->parent());
    }

    return false;
}

static void syncSubsurfaces(LSurface *surface)
{
    for (LSurface *c : surface->children())
        if (c->subsurface())
            RSubsurface::set_sync(nullptr, c->subsurface()->resource()->resource());
}

void RSubsurface::set_sync(wl_client */*client*/, wl_resource *resource)
{
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };

    if (!subsurfaceRole->isSynced())
    {
        subsurfaceRole->m_isSynced = true;
        subsurfaceRole->syncModeChanged();
        syncSubsurfaces(subsurfaceRole->surface());
    }
}

void RSubsurface::set_desync(wl_client */*client*/, wl_resource *resource)
{
    LSubsurfaceRole *subsurfaceRole { static_cast<RSubsurface*>(wl_resource_get_user_data(resource))->subsurfaceRole() };

    if (subsurfaceRole->isSynced() && !hasSyncParent(subsurfaceRole->surface()))
    {
        subsurfaceRole->m_isSynced = false;
        subsurfaceRole->syncModeChanged();

        if (subsurfaceRole->m_hasCache)
        {
            subsurfaceRole->m_hasCache = false;
            RSurface::apply_commit(subsurfaceRole->surface());
        }
    }
}
