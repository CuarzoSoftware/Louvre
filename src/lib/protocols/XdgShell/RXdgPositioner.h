#ifndef RXDGPOSITIONER_H
#define RXDGPOSITIONER_H

#include <LResource.h>
#include <LPositioner.h>

class Louvre::Protocols::XdgShell::RXdgPositioner final : public LResource
{
public:

    const LPositioner &positioner() const noexcept
    {
        return m_positioner;
    }

    bool validate();

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource) noexcept;
    static void set_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height) noexcept;
    static void set_anchor_rect(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height) noexcept;
    static void set_anchor(wl_client *client, wl_resource *resource, UInt32 anchor) noexcept;
    static void set_gravity(wl_client *client, wl_resource *resource, UInt32 gravity) noexcept;
    static void set_constraint_adjustment(wl_client *client, wl_resource *resource, UInt32 constraintAdjustment) noexcept;
    static void set_offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y) noexcept;

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    static void set_reactive(wl_client *client, wl_resource *resource);
    static void set_parent_size(wl_client *client, wl_resource *resource, Int32 parent_width, Int32 parent_height);
    static void set_parent_configure(wl_client *client, wl_resource *resource, UInt32 serial);
#endif

private:
    friend class Louvre::Protocols::XdgShell::GXdgWmBase;
    friend class Louvre::Protocols::XdgShell::RXdgPopup;
    RXdgPositioner(GXdgWmBase *xdgWmBaseRes, UInt32 id) noexcept;
    ~RXdgPositioner() noexcept = default;
    LPositioner m_positioner;
};

#endif // RXDGPOSITIONER_H
