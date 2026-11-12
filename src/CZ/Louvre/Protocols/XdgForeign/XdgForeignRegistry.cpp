#include <CZ/Louvre/Protocols/XdgForeign/XdgForeignRegistry.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>

#include <format>
#include <random>
#include <unordered_map>

using namespace CZ;
using namespace CZ::Protocols::XdgForeign;

/*
 * Handles are secret, unguessable strings. The registry maps them to the
 * live RXdgExportedBase. Every participant lives on the main thread
 * (client dispatch and role destruction both), so no lock is needed.
 */
static std::unordered_map<std::string, RXdgExportedBase *> &registry() noexcept
{
    static std::unordered_map<std::string, RXdgExportedBase *> map;
    return map;
}

/* Handles are capabilities: mint them from the OS CSPRNG (std::random_device
 * is getrandom-backed on Linux). A seeded userspace PRNG would let any client
 * recover the state from ~312 observed handles and predict every other
 * client's handles. random_device yields 32 bits per draw. */
static UInt64 rand64() noexcept
{
    static std::random_device device;
    return (UInt64(device()) << 32) | device();
}

std::string XdgForeignRegistryInternal::MintHandle() noexcept
{
    std::string handle;

    do
    {
        handle = std::format("{:016x}{:016x}", rand64(), rand64());
    } while (registry().find(handle) != registry().end());

    return handle;
}

RXdgExportedBase *XdgForeignRegistryInternal::FindExported(const std::string &handle) noexcept
{
    const auto it { registry().find(handle) };

    if (it == registry().end())
        return nullptr;

    return it->second;
}

void XdgForeignRegistryInternal::RegisterExported(RXdgExportedBase *exported) noexcept
{
    registry().emplace(exported->handle(), exported);
}

void XdgForeignRegistryInternal::UnregisterExported(RXdgExportedBase *exported) noexcept
{
    registry().erase(exported->handle());
}

/******************** RXdgExportedBase ********************/

RXdgExportedBase::RXdgExportedBase
    (
        wl_client *client,
        const wl_interface *iface,
        Int32 version,
        UInt32 id,
        LToplevelRole *toplevel,
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
    m_handle(XdgForeignRegistryInternal::MintHandle()),
    m_toplevel(toplevel)
{
    m_toplevel.setOnDestroyCallback([this](LToplevelRole *) noexcept
    {
        // The exported toplevel is gone: every importer must observe its
        // `destroyed` event. The exported object itself stays alive (the
        // client still owns it) but further imports of the handle are inert.
        notifyImportersDestroyed();
    });

    XdgForeignRegistryInternal::RegisterExported(this);
}

RXdgExportedBase::~RXdgExportedBase() noexcept
{
    notifyImportersDestroyed();
    XdgForeignRegistryInternal::UnregisterExported(this);
}

void RXdgExportedBase::addImporter(RXdgImportedBase *importer) noexcept
{
    m_importers.emplace_back(importer);
}

void RXdgExportedBase::removeImporter(RXdgImportedBase *importer) noexcept
{
    for (auto it { m_importers.begin() }; it != m_importers.end(); ++it)
    {
        if (*it == importer)
        {
            m_importers.erase(it);
            return;
        }
    }
}

void RXdgExportedBase::notifyImportersDestroyed() noexcept
{
    // exportDestroyed() removes itself from m_importers: iterate a copy.
    const auto importers { m_importers };
    m_importers.clear();

    for (auto *importer : importers)
        importer->exportDestroyed();
}

/******************** RXdgImportedBase ********************/

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
        m_exported->addImporter(this);
}

RXdgImportedBase::~RXdgImportedBase() noexcept
{
    dismiss();

    if (m_exported)
        m_exported->removeImporter(this);
}

void RXdgImportedBase::setParentOf(LToplevelRole *child) noexcept
{
    if (m_child.get() == child)
        return;

    if (m_child.get())
        m_child.get()->setParent(nullptr);

    m_child = child;

    if (!child || !m_exported || !m_exported->isValid())
        return;

    /* Refuse cycles: the exported toplevel's ancestor chain must not
     * contain the child, or parenting the child under it would make that
     * chain reach back to itself. A client can export its own toplevel
     * and import it, and two clients can import each other's handles. */
    for (LSurface *s { m_exported->toplevel()->surface() }; s; s = s->parent())
    {
        if (s == child->surface())
            return;
    }

    child->setParent(m_exported->toplevel());
}

void RXdgImportedBase::dismiss() noexcept
{
    if (m_child.get())
    {
        m_child.get()->setParent(nullptr);
        m_child = CZWeak<LToplevelRole>(nullptr);
    }
}

void RXdgImportedBase::exportDestroyed() noexcept
{
    dismiss();
    m_exported = nullptr;
    sendDestroyed();
}
