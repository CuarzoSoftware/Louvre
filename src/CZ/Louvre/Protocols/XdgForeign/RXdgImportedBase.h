#ifndef RXDGIMPORTEDBASE_H
#define RXDGIMPORTEDBASE_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

/**
 * @brief An imported toplevel (xdg-foreign `zxdg_imported_v1` / `zxdg_imported_v2`).
 *
 * Shared by RXdgImportedV1 and RXdgImportedV2. Through `set_parent_of` the importing client makes
 * one of its toplevels a child of the exported toplevel (see LToplevelRole::foreignParent() and
 * LToplevelRole::foreignParentFilter()).
 *
 * When the handle is invalid (unknown, or its export or exported toplevel is gone) the `destroyed`
 * event is sent and the relationship set up, if any, is removed.
 */
class CZ::Protocols::XdgForeign::RXdgImportedBase : public LResource
{
public:
    /// The export it refers to (`nullptr` if the handle is no longer valid).
    RXdgExportedBase *exported() const noexcept { return m_exported; }

    /// The toplevel made a child of the exported one, if any.
    LToplevelRole *child() const noexcept { return m_child.get(); }

protected:
    /// A `nullptr` @p exported means the handle was invalid: each version sends `destroyed` then.
    RXdgImportedBase(wl_client *client, const wl_interface *iface, Int32 version, UInt32 id, RXdgExportedBase *exported, const void *imp) noexcept;
    ~RXdgImportedBase() noexcept;

    /**
     * @brief Makes @p child a child of the exported toplevel (`set_parent_of`).
     *
     * Replaces the previous child, if any. Ignored if the handle is no longer valid, if it would
     * create a cycle, or if LToplevelRole::foreignParentFilter() denies it.
     */
    void setParentOf(LToplevelRole *child) noexcept;

    /// Sends the version's `destroyed` event.
    virtual void sendDestroyed() noexcept = 0;

private:
    friend class RXdgExportedBase;

    /// The export is gone: removes the relationship and sends `destroyed`.
    void exportInvalidated() noexcept;

    /// Removes the relationship (only if the child's parent is still the exported toplevel).
    void unparentChild() noexcept;

    RXdgExportedBase *m_exported { nullptr };
    CZWeak<LToplevelRole> m_child;
};

#endif // RXDGIMPORTEDBASE_H
