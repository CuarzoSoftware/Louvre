#ifndef RFOREIGNTOPLEVELHANDLE_H
#define RFOREIGNTOPLEVELHANDLE_H

#include <LForeignToplevelController.h>
#include <LResource.h>
#include <LWeak.h>
#include <string>
#include <memory>

class Louvre::Protocols::ForeignToplevelManagement::RForeignToplevelHandle final : public LResource
{
public:
    GForeignToplevelManager *foreignToplevelManagerRes() const noexcept { return m_foreignToplevelManagerRes; }
    LToplevelRole *toplevelRole() const noexcept { return m_toplevelRole; }

    LForeignToplevelController *controller() const noexcept
    {
        return m_controller.get();
    }

    bool changed() const noexcept
    {
        return m_changed;
    }

    void updateState() noexcept;

    /******************** REQUESTS ********************/

    static void set_maximized(wl_client *client, wl_resource *resource);
    static void unset_maximized(wl_client *client, wl_resource *resource);
    static void set_minimized(wl_client *client, wl_resource *resource);
    static void unset_minimized(wl_client *client, wl_resource *resource);
    static void activate(wl_client *client, wl_resource *resource, wl_resource *seat);
    static void close(wl_client *client, wl_resource *resource);
    static void set_rectangle(wl_client *client, wl_resource *resource, wl_resource *surface, Int32 x, Int32 y, Int32 width, Int32 height);
    static void destroy(wl_client *client, wl_resource *resource);

#if LOUVRE_FOREIGN_TOPLEVEL_MANAGER_VERSION >= 2
    static void set_fullscreen(wl_client *client, wl_resource *resource, wl_resource *output);
    static void unset_fullscreen(wl_client *client, wl_resource *resource);
#endif

    /******************** EVENTS ********************/

    // Since 1
    void title(const std::string &title) noexcept;
    void appId(const std::string &appId) noexcept;
    void outputEnter(LOutput *output) noexcept;
    void outputLeave(LOutput *output) noexcept;
    void state(wl_array *state) noexcept;
    void done() noexcept;
    void closed() noexcept;

    // Since 3
    bool parent(RForeignToplevelHandle *parent) noexcept;

private:
    friend class GForeignToplevelManager;
    RForeignToplevelHandle(GForeignToplevelManager &foreignToplevelManagerRes, LToplevelRole &toplevelRole);
    ~RForeignToplevelHandle();
    LWeak<GForeignToplevelManager> m_foreignToplevelManagerRes;
    LWeak<LToplevelRole> m_toplevelRole;
    std::unique_ptr<LForeignToplevelController> m_controller;
    bool m_changed { false };
};

#endif // RFOREIGNTOPLEVELHANDLE_H
