#ifndef RBACKGROUNDBLUR_H
#define RBACKGROUNDBLUR_H

#include <LBackgroundBlur.h>
#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::BackgroundBlur::RBackgroundBlur final : public LResource
{
public:
    Wayland::RSurface *surfaceRes() const noexcept { return m_surfaceRes; }

    void state(LBackgroundBlur::State state) noexcept;
    void style(LBackgroundBlur::Style style) noexcept;
    void configure(UInt32 serial) noexcept;

    /******************** REQUESTS ********************/

    static void destroy(wl_client *client, wl_resource *resource);
    static void ack_configure(wl_client *client, wl_resource *resource, UInt32 serial);
    static void set_region(wl_client *client, wl_resource *resource, wl_resource *region);
    static void set_round_rect(wl_client *client, wl_resource *resource,
        Int32 x, Int32 y, Int32 width, Int32 height,
        Int32 radTL, Int32 radTR, Int32 radBR, Int32 radBL);
    static void set_path(wl_client *client, wl_resource *resource, wl_resource *svgPath);
private:
    friend class GBackgroundBlurManager;
    RBackgroundBlur(Wayland::RSurface *surfaceRes, UInt32 id, Int32 version) noexcept;
    ~RBackgroundBlur() noexcept;
    LWeak<Wayland::RSurface> m_surfaceRes;
};

#endif // RBACKGROUNDBLUR_H
