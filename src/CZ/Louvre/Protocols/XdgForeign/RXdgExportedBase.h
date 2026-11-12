#ifndef RXDGEXPORTEDBASE_H
#define RXDGEXPORTEDBASE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <string>
#include <vector>

/**
 * @brief An exported toplevel (xdg-foreign `zxdg_exported_v1` / `zxdg_exported_v2`).
 *
 * Shared by RXdgExportedV1 and RXdgExportedV2. Exporting gives the client a secret, unguessable
 * handle it can pass to another client, which imports it (RXdgImportedBase) to make its own
 * toplevels children of the exported one (e.g. a portal dialog over the app that opened it).
 *
 * The handle stays valid until this object or its toplevel is destroyed. Then every import of it
 * receives its `destroyed` event, and later imports of the handle are invalid.
 *
 * An export without a toplevel (a v1 export of a surface without the xdg_toplevel role, for which
 * v1 defines no protocol error) is inert: it has no handle and can't be imported.
 */
class CZ::Protocols::XdgForeign::RXdgExportedBase : public LResource
{
public:
    /**
     * @brief The live export with the given @p handle.
     *
     * @return `nullptr` if there's none, or its toplevel was destroyed.
     */
    static RXdgExportedBase *Find(const std::string &handle) noexcept;

    /// The handle sent to the client (empty if inert).
    const std::string &handle() const noexcept { return m_handle; }

    /// The exported toplevel (`nullptr` once destroyed, or if inert).
    LToplevelRole *toplevel() const noexcept { return m_toplevel.get(); }

protected:
    /// Mints and publishes the handle (the `handle` event is sent by each version).
    RXdgExportedBase(wl_client *client, const wl_interface *iface, Int32 version, UInt32 id, LToplevelRole *toplevel, const void *imp) noexcept;
    ~RXdgExportedBase() noexcept;

private:
    friend class RXdgImportedBase;

    /// The handle is no longer valid: unpublishes it and invalidates every import of it.
    void invalidate() noexcept;

    std::string m_handle;
    CZWeak<LToplevelRole> m_toplevel;
    std::vector<RXdgImportedBase*> m_importers;
    bool m_published { false };
};

#endif // RXDGEXPORTEDBASE_H
