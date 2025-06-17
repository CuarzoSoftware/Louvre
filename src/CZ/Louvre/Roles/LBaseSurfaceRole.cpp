#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Roles/LBaseSurfaceRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <cassert>

using namespace Louvre;

LBaseSurfaceRole::LBaseSurfaceRole(Type type, LResource *resource, LSurface *surface, UInt32 roleId) noexcept :
    LFactoryObject(type),
    m_surface(surface),
    m_resource(resource),
    m_roleId(roleId)
{
    surface->imp()->setRole(this);
}

LBaseSurfaceRole::~LBaseSurfaceRole()
{
    assert(m_destructorValidated && "Missing validateDestructor() call in role destructor");
    notifyDestruction();
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

void LBaseSurfaceRole::validateDestructor() noexcept
{
    assert(surface() != nullptr && "The surface() reference must remain valid until the user role destructor is called.");
    assert(surface()->role() == nullptr && "The surface() should not have any role assigned (roleChanged() should be called before the role destructor).");
    assert(!surface()->mapped() && "The surface() should not be mapped here and in the Louvre specific role destructor.");
    m_destructorValidated = true;
}
