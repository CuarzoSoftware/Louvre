#ifndef XDGFOREIGNREGISTRY_H
#define XDGFOREIGNREGISTRY_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <string>
#include <vector>

namespace CZ
{
    class LToplevelRole;
}

namespace CZ::Protocols::XdgForeign {

// Shared machinery for the xdg-foreign-unstable-v1/v2 protocols.
//
// RXdgExportedBase owns one handle for an exported xdg_toplevel and
// tracks every importer that resolved it. RXdgImportedBase parents a
// client toplevel to the exported one via LToplevelRole::setParent()
// (client-agnostic) and fires its `destroyed` event when the export or
// the exported toplevel goes away.

class RXdgImportedBase;

class RXdgExportedBase : public LResource
{
public:

    // Mints the handle, publishes it in the registry and watches the
    // toplevel. The handle event itself is sent by each version's ctor.
    RXdgExportedBase(wl_client *client, const wl_interface *iface, Int32 version, UInt32 id, LToplevelRole *toplevel, const void *imp) noexcept;
    ~RXdgExportedBase() noexcept override;

    const std::string &handle() const noexcept { return m_handle; }

    // True while the exported toplevel is still alive.
    bool isValid() const noexcept { return m_toplevel.get() != nullptr; }

    LToplevelRole *toplevel() const noexcept { return m_toplevel.get(); }

private:
    friend class RXdgImportedBase;

    void addImporter(RXdgImportedBase *importer) noexcept;
    void removeImporter(RXdgImportedBase *importer) noexcept;
    void notifyImportersDestroyed() noexcept;

    std::string m_handle;
    CZWeak<LToplevelRole> m_toplevel;
    std::vector<RXdgImportedBase *> m_importers;
};

class RXdgImportedBase : public LResource
{
public:

    // A nullptr exported means the handle was unknown: the object stays
    // inert (per spec, no `destroyed` event is pending for it).
    RXdgImportedBase(wl_client *client, const wl_interface *iface, Int32 version, UInt32 id, RXdgExportedBase *exported, const void *imp) noexcept;
    ~RXdgImportedBase() noexcept override;

    // Shared set_parent_of() implementation. A nullptr child detaches.
    void setParentOf(LToplevelRole *child) noexcept;

    // Shared dismiss() implementation.
    void dismiss() noexcept;

protected:

    // Called when the export (or its toplevel) is gone: send the
    // version-specific `destroyed` event.
    virtual void sendDestroyed() noexcept = 0;

private:
    friend class RXdgExportedBase;

    void exportDestroyed() noexcept;

    RXdgExportedBase *m_exported { nullptr };
    CZWeak<LToplevelRole> m_child;
};

// Registry of live exported handles (compositor-wide, main thread only).
namespace XdgForeignRegistryInternal {
    RXdgExportedBase *FindExported(const std::string &handle) noexcept;
    void RegisterExported(RXdgExportedBase *exported) noexcept;
    void UnregisterExported(RXdgExportedBase *exported) noexcept;
    std::string MintHandle() noexcept;
}
}

#endif // XDGFOREIGNREGISTRY_H
