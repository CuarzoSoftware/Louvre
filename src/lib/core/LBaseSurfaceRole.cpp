#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LCompositor.h>

using namespace Louvre;

LBaseSurfaceRole::LBaseSurfaceRole(LResource *resource, LSurface *surface, UInt32 roleId) : LPRIVATE_INIT_UNIQUE(LBaseSurfaceRole)
{
    imp()->resource = resource;
    imp()->surface = surface;
    imp()->roleId = roleId;
}

LBaseSurfaceRole::~LBaseSurfaceRole()
{
    if (imp()->surface)
    {
        LSurface *surface = imp()->surface;
        imp()->surface = nullptr;
        surface->imp()->setPendingRole(nullptr);
        surface->imp()->applyPendingRole();
        surface->imp()->setMapped(false);
    }   
}

UInt32 LBaseSurfaceRole::roleId() const
{
    return imp()->roleId;
}

LSurface *LBaseSurfaceRole::surface() const
{
    return imp()->surface;
}

LResource *LBaseSurfaceRole::resource() const
{
    return imp()->resource;
}

bool LBaseSurfaceRole::acceptCommitRequest(Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);
    return true;
}

void LBaseSurfaceRole::handleSurfaceCommit(Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);

    /* No default implementation */
}

void LBaseSurfaceRole::handleSurfaceBufferAttach(wl_resource *buffer, Int32 x, Int32 y)
{
    L_UNUSED(buffer);
    L_UNUSED(x);
    L_UNUSED(y);

    /* No default implementation */
}

void LBaseSurfaceRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    L_UNUSED(x);
    L_UNUSED(y);

    /* No default implementation */
}

void LBaseSurfaceRole::handleParentCommit()
{
    /* No default implementation */
}

void LBaseSurfaceRole::handleParentMappingChange()
{
    /* No default implementation */
}

void LBaseSurfaceRole::handleParentChange()
{
    /* No default implementation */
}
