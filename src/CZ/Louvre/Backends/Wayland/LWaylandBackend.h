#ifndef LWAYLANDBACKEND_H
#define LWAYLANDBACKEND_H

#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/Backends/Wayland/xdg-shell-client.h>
#include <CZ/Louvre/LLog.h>

class CZ::LWaylandBackend : public LBackend
{
public:

    struct WLSeat
    {
        wl_seat *proxy;
        wl_pointer *pointer;
        wl_keyboard *keyboard;
        wl_touch *touch;
        UInt32 name;
        UInt32 pointerEnterSerial;
    };

    struct WL
    {
        wl_display *display;
        wl_registry *registry;
        wl_surface *cursorSurface;
        wl_surface *surface;
        xdg_wm_base *xdgWmBase;
        xdg_surface *xdgSurface;
        xdg_toplevel *xdgToplevel;
        wl_compositor *compositor;
        wl_callback *callback;
        WLSeat seat;
    };

    LWaylandBackend() noexcept : LBackend(LBackendId::Wayland) {};

    /* Common */

    bool init() noexcept override;
    void unit() noexcept override;
    void suspend() noexcept override {};
    void resume() noexcept override {};

    /* Input */

    const std::set<std::shared_ptr<CZInputDevice>> &inputDevices() const noexcept override { return m_inputDevices; };
    void inputSetLeds(UInt32 /*leds*/) noexcept override {};
    void inputForceUpdate() noexcept override {};

    /* Output */

    const std::vector<LOutput*> &outputs() const noexcept override { return m_outputs; };
    std::shared_ptr<LDMAFeedback> defaultFeedback() const noexcept override { return m_defaultFeedback; };
    clockid_t presentationClock() const noexcept override { return CLOCK_MONOTONIC; };
    std::shared_ptr<SRMLease> createLease(const std::unordered_set<LOutput*> &) noexcept override { return {}; };

    WL wl {};
protected:
    friend class LWaylandOutput;
    bool initDisplay() noexcept;
    bool initReam() noexcept;
    bool initOutput() noexcept;
    void initDefaultFeedback() noexcept;
    std::shared_ptr<RCore> m_ream;
    std::shared_ptr<CZEventSource> m_source;
    std::vector<LOutput*> m_outputs;
    std::set<std::shared_ptr<CZInputDevice>> m_inputDevices;
    std::shared_ptr<LDMAFeedback> m_defaultFeedback;
    CZLogger log { LLog.newWithContext("Wayland Backend") };
};

#endif // LWAYLANDBACKEND_H
