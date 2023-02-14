#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LPoint.h>
#include <LSurface.h>
#include <LCompositor.h>

using namespace Louvre;

LBaseSurfaceRole::LBaseSurfaceRole(wl_resource *resource, LSurface *surface, UInt32 roleId)
{
    m_baseImp = new LBaseSurfaceRolePrivate();
    m_baseImp->resource = resource;
    m_baseImp->surface = surface;
    m_baseImp->compositor = surface->compositor();
    m_baseImp->roleId = roleId;
}

LBaseSurfaceRole::~LBaseSurfaceRole()
{
    if(m_baseImp->surface)
    {
        m_baseImp->surface->imp()->setPendingRole(nullptr);
        m_baseImp->surface->imp()->applyPendingRole();
    }

    delete m_baseImp;
}

UInt32 LBaseSurfaceRole::roleId()
{
    return m_baseImp->roleId;
}

LCompositor *LBaseSurfaceRole::compositor() const
{
    return m_baseImp->compositor;
}

LSurface *LBaseSurfaceRole::surface() const
{
    return m_baseImp->surface;
}

LSeat *LBaseSurfaceRole::seat() const
{
    return compositor()->seat();
}

wl_resource *LBaseSurfaceRole::resource() const
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
}

bool LBaseSurfaceRole::acceptCommitRequest(Protocols::Wayland::SurfaceResource::CommitOrigin origin)
{
    L_UNUSED(origin);
    return true;
}

void LBaseSurfaceRole::handleSurfaceCommit()
{
    /* Empty */
}

void LBaseSurfaceRole::handleSurfaceBufferAttach(wl_resource *buffer, Int32 x, Int32 y)
{
    L_UNUSED(buffer);
    L_UNUSED(x);
    L_UNUSED(y);
}

void LBaseSurfaceRole::handleSurfaceOffset(Int32 x, Int32 y)
{
    L_UNUSED(x);
    L_UNUSED(y);
}

void LBaseSurfaceRole::handleParentCommit()
{

}

void LBaseSurfaceRole::handleParentMappingChange()
{
    /* Empty */
}

void LBaseSurfaceRole::handleParentChange()
{

}
