#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LCompositor.h>

using namespace Louvre;

LBaseSurfaceRole::LBaseSurfaceRole(LResource *resource, LSurface *surface, UInt32 roleId)
{
    m_baseImp = new LBaseSurfaceRolePrivate();
    m_baseImp->resource = resource;
    m_baseImp->surface = surface;
    m_baseImp->roleId = roleId;
}

LBaseSurfaceRole::~LBaseSurfaceRole()
{
    if (m_baseImp->surface)
    {
        m_baseImp->surface->imp()->setPendingRole(nullptr);
        m_baseImp->surface->imp()->applyPendingRole();
    }

    delete m_baseImp;
}

UInt32 LBaseSurfaceRole::roleId() const
{
    return m_baseImp->roleId;
}

LSurface *LBaseSurfaceRole::surface() const
{
    return m_baseImp->surface;
}

LResource *LBaseSurfaceRole::resource() const
{
    return m_baseImp->resource;
}

LBaseSurfaceRole::LBaseSurfaceRolePrivate *LBaseSurfaceRole::baseImp() const
{
    return m_baseImp;
}

void LBaseSurfaceRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);
    L_UNUSED(newScale);

    /* No default implementation */
}

bool LBaseSurfaceRole::acceptCommitRequest(Protocols::Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);
    return true;
}

void LBaseSurfaceRole::handleSurfaceCommit()
{
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
