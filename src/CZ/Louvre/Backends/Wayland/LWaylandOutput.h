#ifndef LWAYLANDOUTPUT_H
#define LWAYLANDOUTPUT_H

#include <CZ/Louvre/Backends/Wayland/xdg-shell-client.h>
#include <CZ/Ream/WL/RWLSwapchain.h>
#include <CZ/Louvre/Backends/LBackendOutput.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>

namespace CZ
{
class LWaylandOutput : public LBackendOutput
{
public:
    static LOutput *Make(LWaylandBackend *backend) noexcept;
    ~LWaylandOutput() noexcept;

    bool init() noexcept override;
    bool repaint() noexcept override;
    void unit() noexcept override;

    const std::string &name() const noexcept override;
    const std::string &make() const noexcept override;
    const std::string &model() const noexcept override;
    const std::string &desc() const noexcept override;
    const std::string &serial() const noexcept override;

    SkISize mmSize() const noexcept override { return {0,0}; }
    RSubpixel subpixel() const noexcept override { return RSubpixel::Unknown; }
    bool isNonDesktop() const noexcept override { return false; };
    UInt32 id() const noexcept override { return 1; };
    RDevice *device() const noexcept override;

    void setContentType(RContentType /*type*/) noexcept override {};
    RContentType contentType() const noexcept override { return RContentType::Graphics; };

    /* Gamma LUT */

    size_t gammaSize() const noexcept override { return 0; };
    std::shared_ptr<const RGammaLUT> gammaLUT() const noexcept override { return {}; };
    bool setGammaLUT(std::shared_ptr<const RGammaLUT> /*gamma*/) noexcept override { return false; };

    /* Rendering */

    UInt32 imageIndex() const noexcept override { return 0; };
    UInt32 imageAge() const noexcept override { return m_age; }
    const std::vector<std::shared_ptr<RImage>> &images() const noexcept override { return m_images; };
    void setDamage(const SkRegion &region) noexcept override { m_damage = region; }

    /* V-SYNC */

    bool canDisableVSync() const noexcept override { return false; };
    bool isVSyncEnabled() const noexcept override { return true; };
    bool enableVSync(bool /*enabled*/) noexcept override { return false; };
    UInt64 paintEventId() const noexcept override { return m_paintEventId; };

    /* Modes */

    const std::vector<std::shared_ptr<LOutputMode>> &modes() const noexcept override { return m_modes; };
    const std::shared_ptr<LOutputMode> preferredMode() const noexcept override { return m_modes[0]; };
    const std::shared_ptr<LOutputMode> currentMode() const noexcept override { return m_modes[0]; };
    int setMode(std::shared_ptr<LOutputMode> /*mode*/) noexcept override { return 1; };

    /* Cursor */

    bool hasCursor() const noexcept override;
    bool setCursor(UInt8 *pixels) noexcept override;
    bool setCursorPos(SkIPoint pos) noexcept override;

    /**
     * @brief Applies the cursor to the parent compositor's pointer (main thread).
     *
     * The parent draws and moves it: the image is only redrawn when it, its size or the scale
     * changed, and wl_pointer.set_cursor only sent when that or the hotspot changed, or the pointer
     * re-entered (new serial). Never on mere motion.
     */
    void updateCursor() noexcept;

    /// No frame callback for this long (e.g. the window hidden: the parent stops sending them):
    /// presented anyway, so painting (and clients' frame callbacks) goes on, slowly.
    static constexpr UInt32 FrameTimeoutMs { 1000 };

protected:
    friend class LWaylandBackend;
    LWaylandOutput(LWaylandBackend *backend) noexcept;
    bool event(const CZEvent &e) noexcept override;
    static void handle_preferred_buffer_scale(void *data, wl_surface *surface, Int32 factor) noexcept;
    static void handle_xdg_surface_configure(void *data, xdg_surface *xdgSurface, UInt32 serial) noexcept;
    static void handle_xdg_toplevel_configure(void *data, xdg_toplevel *toplevel, Int32 width, Int32 height, wl_array *states) noexcept;
    static void handle_xdg_toplevel_close(void *data, xdg_toplevel *toplevel) noexcept;
    static void handle_callback_done(void *data, wl_callback *callback, UInt32 ms) noexcept;

    /// The frame was shown (frame callback, or FrameTimeoutMs without one). Compositor lock held.
    void framePresented(const CZPresentationTime &info) noexcept;

    /// Wakes the render thread (see repaint(), unit()).
    void wake() noexcept;

    // The render thread sleeps until woken, or until m_frameDeadline while a frame callback is pending
    std::mutex m_wakeMutex;
    std::condition_variable m_wakeCond;
    bool m_wake { false };
    bool m_waitingFrame { false };
    std::chrono::steady_clock::time_point m_frameDeadline;
    CZWeak<LWaylandBackend> m_backend;
    std::vector<std::shared_ptr<RImage>> m_images { {} };
    std::optional<SkRegion> m_damage;
    std::vector<std::shared_ptr<LOutputMode>> m_modes;
    std::shared_ptr<RWLSwapchain> m_swapchain;
    UInt32 m_age { 0 };
    UInt64 m_paintEventId { 0 };
    Int32 m_scale { 1 };
    SkISize m_size { 1200, 800 };
    bool m_configured { false };
    std::atomic<bool> m_pendingRepaint { false };

    std::shared_ptr<RImage> m_cursorImage;
    SkIPoint m_cursorHotspot {};
    std::shared_ptr<RWLSwapchain> m_cursorSwapchain;

    // What the parent compositor was last given (see updateCursor())
    std::shared_ptr<RImage> m_cursorDrawnImage;
    SkISize m_cursorDrawnSize {};
    SkIPoint m_cursorSentHotspot {};
    UInt32 m_cursorSentSerial { 0 };
    bool m_cursorSent { false };     // A cursor image (not none) set
    bool m_cursorRedraw { true };    // setCursor() since: the image's content may have changed
    std::atomic<bool> m_cursorScaleChanged { false }; // Set by the render thread, applied on the main one

    std::optional<std::promise<bool>> m_unitPromise;
};
}

#endif // LWAYLANDOUTPUT_H
