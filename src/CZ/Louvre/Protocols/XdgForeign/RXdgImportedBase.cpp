#include <CZ/Louvre/Protocols/XdgForeign/RXdgImportedBase.h>
#include <CZ/Louvre/Protocols/XdgForeign/RXdgExportedBase.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <algorithm>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

RXdgImportedBase::RXdgImportedBase
    (
        wl_client *client,
        const wl_interface *iface,
        Int32 version,
        UInt32 id,
        RXdgExportedBase *exported,
        const void *imp
    ) noexcept
    :LResource
    (
        client,
        iface,
        version,
        id,
        imp
    ),
    m_exported(exported)
{
    if (m_exported)
        m_exported->m_importers.emplace_back(this);
}

RXdgImportedBase::~RXdgImportedBase() noexcept
{
    // Destroying the imported object invalidates the relationship it set up.
    unparentChild();

    if (m_exported)
        std::erase(m_exported->m_importers, this);
}

void RXdgImportedBase::setParentOf(LToplevelRole *child) noexcept
{
    // Replaces the previous relationship (also when asked again for the same child, e.g. after it
    // was lost because a toplevel was unmapped).
    unparentChild();

    LToplevelRole *parent { m_exported ? m_exported->toplevel() : nullptr };

    if (!child || !parent)
        return;

    // No cycles: the parent's ancestors must not include the child (e.g. a client importing its
    // own export, or two clients importing each other's handles).
    for (LSurface *s { parent->surface() }; s; s = s->parent())
        if (s == child->surface())
            return;

    if (!child->foreignParentFilter(parent))
        return;

    m_child.reset(child);
    child->m_foreignParent.reset(parent);
    child->setParent(parent);
}

void RXdgImportedBase::unparentChild() noexcept
{
    LToplevelRole *child { m_child.get() };
    m_child.reset();

    if (!child)
        return;

    LToplevelRole *parent { m_exported ? m_exported->toplevel() : nullptr };

    // Only if it's still the exported toplevel's child (the client may have set another parent
    // with xdg_toplevel.set_parent since).
    if (parent && child->surface()->parent() == parent->surface())
        child->setParent(nullptr);

    if (child->m_foreignParent.get() == parent)
        child->m_foreignParent.reset();
}

void RXdgImportedBase::exportInvalidated() noexcept
{
    unparentChild();
    m_exported = nullptr;
    sendDestroyed();
}
