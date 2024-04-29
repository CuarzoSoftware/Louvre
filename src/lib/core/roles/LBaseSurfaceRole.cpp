#include <private/LSurfacePrivate.h>
#include <LBaseSurfaceRole.h>
#include <LCompositor.h>

using namespace Louvre;

LBaseSurfaceRole::LBaseSurfaceRole(Type type, LResource *resource, LSurface *surface, UInt32 roleId) noexcept :
    LFactoryObject(type),
    m_surface(surface),
    m_resource(resource),
    m_roleId(roleId)
{}

LBaseSurfaceRole::~LBaseSurfaceRole()
{
    if (surface())
    {
        auto &tmp { *surface() };
        m_surface.reset();
        tmp.imp()->setPendingRole(nullptr);
        tmp.imp()->applyPendingRole();
        tmp.imp()->setMapped(false);
    }   
}

LClient *LBaseSurfaceRole::client() const noexcept
{
    return resource() == nullptr ? nullptr : resource()->client();
}

bool LBaseSurfaceRole::acceptCommitRequest(CommitOrigin origin)
{
    L_UNUSED(origin);
    return true;
}

void LBaseSurfaceRole::handleSurfaceCommit(CommitOrigin origin)
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
