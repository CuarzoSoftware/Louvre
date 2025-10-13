#ifndef RXDGTOPLEVEL_H
#define RXDGTOPLEVEL_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>
#include <memory>

class CZ::Protocols::XdgShell::RXdgToplevel final : public LResource
{
public:

    RXdgSurface *xdgSurfaceRes() const noexcept
    {
        return m_xdgSurfaceRes;
    }

    LToplevelRole *toplevelRole() const noexcept
    {
        return m_toplevelRole.get();
    }

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void set_parent(wl_client *client, wl_resource *resource, wl_resource *parent);
    static void set_title(wl_client *client, wl_resource *resource, const char *title);
    static void set_app_id(wl_client *client, wl_resource *resource, const char *app_id);
    static void show_window_menu(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, Int32 x, Int32 y);
    static void move(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial);
    static void resize(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial, UInt32 edges);
    static void set_max_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height);
    static void set_min_size(wl_client *client, wl_resource *resource, Int32 width, Int32 height);
    static void set_maximized(wl_client *client, wl_resource *resource);
    static void unset_maximized(wl_client *client, wl_resource *resource);
    static void set_fullscreen(wl_client *client, wl_resource *resource, wl_resource *output);
    static void unset_fullscreen(wl_client *client, wl_resource *resource);
    static void set_minimized(wl_client *client, wl_resource *resource);

    /******************** EVENTS ********************/

    // Since 1
    void configure(const SkISize &size, wl_array *states) noexcept;
    void close() noexcept;

    // Since 4
    bool configureBounds(const SkISize &bounds) noexcept;

    // Since 5
    bool wmCapabilities(wl_array *capabilities) noexcept;

private:
    friend class CZ::Protocols::XdgShell::RXdgSurface;
    RXdgToplevel(RXdgSurface *xdgSurfaceRes, UInt32 id);
    ~RXdgToplevel();

    CZWeak<RXdgSurface> m_xdgSurfaceRes;
    std::unique_ptr<LToplevelRole> m_toplevelRole;
};

#endif // RXDGTOPLEVEL_H
