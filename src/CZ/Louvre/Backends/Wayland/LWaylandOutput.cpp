#include <CZ/Louvre/Backends/Wayland/LWaylandOutputMode.h>
#include <CZ/Louvre/Backends/Wayland/LWaylandOutput.h>
#include <CZ/Louvre/Backends/Wayland/LWaylandBackend.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Private/LLockGuard.h>
#include <CZ/Louvre/Cursor/LCursorSource.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Ream/WL/RWLSwapchain.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Core/CZPresentationTime.h>
#include <CZ/Core/CZCore.h>
#include <ctime>

using namespace CZ;

static wl_surface_listener surfaceLis {};
static xdg_surface_listener xdgSurfaceLis {};
static xdg_toplevel_listener xdgToplevelLis {};
static wl_callback_listener callbackLis {};

// The parent compositor's choice (it may refuse server-side decorations): nothing to draw either way
static const zxdg_toplevel_decoration_v1_listener xdgToplevelDecorationLis
{
    .configure = [](void */*data*/, zxdg_toplevel_decoration_v1 */*decoration*/, UInt32 mode)
    {
        LLog(CZDebug, "[Wayland Backend] Window decorations: {}",
             mode == ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE ? "server-side" : "client-side (none drawn)");
    }
};

void LWaylandOutput::handle_preferred_buffer_scale(void *data, wl_surface */*surface*/, Int32 factor) noexcept
{
    auto *o { static_cast<LWaylandOutput*>(data) };

    if (o->m_scale != factor)
    {
        o->m_scale = factor;

        if (!o->m_unitPromise.has_value())
            o->repaint();
    }
}

void LWaylandOutput::handle_xdg_surface_configure(void *data, xdg_surface *xdgSurface, UInt32 serial) noexcept
{
    auto *o { static_cast<LWaylandOutput*>(data) };
    xdg_surface_ack_configure(xdgSurface, serial);
    o->m_configured = true;

    if (!o->m_unitPromise.has_value())
        o->repaint();
}

void LWaylandOutput::handle_xdg_toplevel_configure(void *data, xdg_toplevel */*toplevel*/, Int32 width, Int32 height, wl_array */*states*/) noexcept
{
    auto *o { static_cast<LWaylandOutput*>(data) };
    o->m_size.fWidth = width == 0 ? o->m_size.fWidth : width;
    o->m_size.fHeight = height == 0 ? o->m_size.fHeight : height;
}

void LWaylandOutput::handle_xdg_toplevel_close(void */*data*/, xdg_toplevel */*toplevel*/) noexcept
{
    compositor()->finish();
}

void LWaylandOutput::handle_callback_done(void *data, wl_callback *callback, UInt32 ms) noexcept
{
    auto *o { static_cast<LWaylandOutput*>(data) };

    CZPresentationTime timeInfo {};
    timeInfo.seq = timeInfo.paintEventId = o->m_paintEventId;
    timeInfo.time.tv_sec = ms / 1000;
    timeInfo.time.tv_nsec = (ms % 1000) * 1000000;

    wl_callback_destroy(callback);
    o->m_backend->wl.callback = nullptr;
    o->framePresented(timeInfo);
}

void LWaylandOutput::framePresented(const CZPresentationTime &info) noexcept
{
    output()->imp()->backendPresented(info);

    {
        std::lock_guard wakeLock { m_wakeMutex };
        m_waitingFrame = false;
    }

    if (m_unitPromise.has_value())
        return;

    if (m_pendingRepaint)
        wake();
}

void LWaylandOutput::wake() noexcept
{
    {
        std::lock_guard wakeLock { m_wakeMutex };
        m_wake = true;
    }

    m_wakeCond.notify_one();
}

LOutput *LWaylandOutput::Make(LWaylandBackend *backend) noexcept
{
    auto *o { new LWaylandOutput(backend) };
    o->m_modes.emplace_back(std::shared_ptr<LWaylandOutputMode>(new LWaylandOutputMode(o)));

    surfaceLis.preferred_buffer_scale = &handle_preferred_buffer_scale;
    surfaceLis.preferred_buffer_transform = [](auto, auto, auto){};
    surfaceLis.enter = [](auto, auto, auto){};
    surfaceLis.leave = [](auto, auto, auto){};
    xdgSurfaceLis.configure = &handle_xdg_surface_configure;
    xdgToplevelLis.configure = &handle_xdg_toplevel_configure;
    xdgToplevelLis.close = &handle_xdg_toplevel_close;
    callbackLis.done = &handle_callback_done;

    auto &wl { backend->wl };
    wl.surface = wl_compositor_create_surface(wl.compositor);
    wl_surface_add_listener(wl.surface, &surfaceLis, o);
    wl.xdgSurface = xdg_wm_base_get_xdg_surface(wl.xdgWmBase, wl.surface);
    xdg_surface_add_listener(wl.xdgSurface, &xdgSurfaceLis, o);
    wl.xdgToplevel = xdg_surface_get_toplevel(wl.xdgSurface);
    xdg_toplevel_add_listener(wl.xdgToplevel, &xdgToplevelLis, o);

    // Named after the compositor's executable (e.g. in the parent's title bar and task switcher)
    xdg_toplevel_set_title(wl.xdgToplevel, program_invocation_short_name);
    xdg_toplevel_set_app_id(wl.xdgToplevel, program_invocation_short_name);

    // Decorated by the parent compositor if it can (before the first commit, see init())
    if (wl.xdgDecorationManager)
    {
        wl.xdgToplevelDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(wl.xdgDecorationManager, wl.xdgToplevel);
        zxdg_toplevel_decoration_v1_add_listener(wl.xdgToplevelDecoration, &xdgToplevelDecorationLis, o);
        zxdg_toplevel_decoration_v1_set_mode(wl.xdgToplevelDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    wl.cursorSurface = wl_compositor_create_surface(wl.compositor);
    o->m_cursorSwapchain = RWLSwapchain::Make(wl.cursorSurface, {64, 64}, true); // Paced: never blocks the main thread

    LOutput::Params params {};
    params.backend.reset(o);
    return LFactory::createObject<LOutput>(&params);
}

LWaylandOutput::~LWaylandOutput() noexcept
{
    auto &wl { m_backend->wl };

    // Before its toplevel (protocol requirement)
    if (wl.xdgToplevelDecoration)
    {
        zxdg_toplevel_decoration_v1_destroy(wl.xdgToplevelDecoration);
        wl.xdgToplevelDecoration = nullptr;
    }

    xdg_toplevel_destroy(wl.xdgToplevel);
    wl.xdgToplevel = nullptr;

    xdg_surface_destroy(wl.xdgSurface);
    wl.xdgSurface = nullptr;

    m_swapchain.reset();

    wl_surface_destroy(wl.surface);
    wl.surface = nullptr;

    m_cursorSwapchain.reset();
    wl_surface_destroy(wl.cursorSurface);
    wl.cursorSurface = nullptr;

    m_cursorImage.reset();
    m_images = {{}};
}

bool LWaylandOutput::init() noexcept
{
    auto *mode { static_cast<LWaylandOutputMode*>(m_modes[0].get()) };
    mode->m_size = { 1200, 800 };
    m_age = 0;
    m_unitPromise.reset();
    m_pendingRepaint = false;
    m_configured = false;
    m_wake = false;
    m_waitingFrame = false;
    xdg_toplevel_set_maximized(m_backend->wl.xdgToplevel);
    wl_surface_attach(m_backend->wl.surface, NULL, 0, 0);
    wl_surface_commit(m_backend->wl.surface);
    wl_display_flush(m_backend->wl.display);

    std::thread([this](){
        auto &wl { m_backend->wl };
        auto *mode { static_cast<LWaylandOutputMode*>(m_modes[0].get()) };
        auto core { CZCore::Get() };
        output()->imp()->backendInitializeGL();
        auto event { std::make_shared<CZPresentationEvent>() };

        while (true)
        {
            // Until woken (repaint(), a frame callback, unit()) or, while a frame is pending, its deadline
            std::chrono::steady_clock::time_point deadline;

            {
                std::unique_lock wakeLock { m_wakeMutex };
                const auto woken = [this]{ return m_wake; };

                if (m_waitingFrame)
                    m_wakeCond.wait_until(wakeLock, m_frameDeadline, woken);
                else
                    m_wakeCond.wait(wakeLock, woken);

                m_wake = false;
                deadline = m_frameDeadline;
            }

            std::optional<RSwapchainImage> image;
            std::optional<SkRegion> damage;

            // Painted with the compositor locked, presented without it (see below)
            {
                LLockGuard lock {};

                if (m_unitPromise.has_value())
                {
                    m_unitPromise->set_value(true);
                    break;
                }

                if (wl.callback)
                {
                    // Not shown yet
                    if (std::chrono::steady_clock::now() < deadline)
                        continue;

                    // No frame callback for too long (e.g. the window is hidden): as if presented
                    wl_callback_destroy(wl.callback);
                    wl.callback = nullptr;
                    CZPresentationTime timeInfo {};
                    timeInfo.seq = timeInfo.paintEventId = m_paintEventId;
                    clock_gettime(CLOCK_MONOTONIC, &timeInfo.time);
                    framePresented(timeInfo);
                }

                if (!m_configured || !m_pendingRepaint)
                    continue;

                const SkISize newSize { m_size.fWidth * m_scale, m_size.fHeight * m_scale };

                wl_surface_set_buffer_scale(wl.surface, m_scale);

                if (!m_swapchain)
                    m_swapchain = RWLSwapchain::Make(wl.surface, newSize, true); // Paced by frame callbacks

                if (output()->transform() != CZTransform::Normal || newSize != mode->m_size || output()->scale() != m_scale)
                {
                    output()->setTransform(CZTransform::Normal);
                    mode->m_size = newSize;
                    m_swapchain->resize(newSize);
                    output()->setScale(m_scale);
                    output()->imp()->backendResizeGL();
                    m_cursorScaleChanged = true; // Applied on the main thread (see event())
                }

                if (auto acquired { m_swapchain->acquire() })
                    image.emplace(*acquired);

                // No free buffer (e.g. a Vulkan acquire timeout): tried again soon
                if (!image)
                {
                    std::lock_guard wakeLock { m_wakeMutex };
                    m_waitingFrame = true;
                    m_frameDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(4);
                    continue;
                }

                m_pendingRepaint = false;
                m_paintEventId++;
                m_images[0] = image->image;
                m_age = image->age;
                output()->imp()->backendPaintGL();
                wl.callback = wl_surface_frame(wl.surface);
                wl_callback_add_listener(wl.callback, &callbackLis, this);

                {
                    std::lock_guard wakeLock { m_wakeMutex };
                    m_waitingFrame = true;
                    m_frameDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(FrameTimeoutMs);
                }

                damage = std::move(m_damage);
                m_damage.reset();
            }

            // Without the compositor lock: it may wait for the parent to release a buffer (e.g.
            // Mesa's EGL), which must not block input, clients or the main thread meanwhile
            m_swapchain->present(*image, damage ? &*damage : nullptr);
            wl_display_flush(wl.display);

            // The main thread dispatches what was read meanwhile (see LWaylandBackend::dispatchEvents())
            core->postEvent(event, *this);
        }

        output()->imp()->backendUninitializeGL();
    }).detach();

    return true;
}

bool LWaylandOutput::repaint() noexcept
{
    m_pendingRepaint = true;
    wake();
    return true;
}

void LWaylandOutput::unit() noexcept
{
    {
        LLockGuard lock {};
        m_unitPromise = std::promise<bool>();
    }

    auto future { m_unitPromise->get_future() };
    wake();
    future.get();

    auto &wl { m_backend->wl };

    if (wl.callback)
    {
        wl_callback_destroy(wl.callback);
        wl.callback = nullptr;
    }

    wl_surface_attach(m_backend->wl.surface, NULL, 0, 0);
    wl_surface_commit(m_backend->wl.surface);
    wl_display_flush(wl.display);
}

const std::string &LWaylandOutput::name() const noexcept
{
    static const std::string str { "WL-1" };
    return str;
}

const std::string &LWaylandOutput::make() const noexcept
{
    static const std::string str { "Cuarzo Software" };
    return str;
}

const std::string &LWaylandOutput::model() const noexcept
{
    static const std::string str { "xdg_toplevel" };
    return str;
}

const std::string &LWaylandOutput::desc() const noexcept
{
    static const std::string str { "Virtual Wayland Output" };
    return str;
}

const std::string &LWaylandOutput::serial() const noexcept
{
    static const std::string str { "0" };
    return str;
}

RDevice *LWaylandOutput::device() const noexcept
{
    return m_backend->m_ream->mainDevice();
}

bool LWaylandOutput::hasCursor() const noexcept
{
    return
        m_backend->wl.cursorSurface != nullptr &&
        m_backend->wl.seat.pointer != nullptr;
}

bool LWaylandOutput::setCursorPos(SkIPoint) noexcept
{
    if (!hasCursor())
        return false;

    // The parent compositor moves it: only the hotspot matters (see updateCursor())
    auto src { cursor()->source() };

    if (src && src->image() && !src->image()->size().isEmpty())
        m_cursorHotspot = SkIPoint(
            (src->hotspot().x() * cursor()->size().width())/SkScalar(src->image()->size().width()),
            (src->hotspot().y() * cursor()->size().height())/SkScalar(src->image()->size().height())
        );

    updateCursor();
    return true;
}

void LWaylandOutput::updateCursor() noexcept
{
    if (!m_cursorSwapchain || !hasCursor())
        return;

    auto &wl { m_backend->wl };
    const UInt32 serial { wl.seat.pointerEnterSerial };

    if (!m_cursorImage)
    {
        if (m_cursorSent || serial != m_cursorSentSerial)
        {
            wl_pointer_set_cursor(wl.seat.pointer, serial, NULL, 0, 0);
            m_cursorSent = false;
            m_cursorSentSerial = serial;
            m_cursorDrawnImage.reset();
        }

        return;
    }

    const SkISize size { cursor()->rect().width() * m_scale, cursor()->rect().height() * m_scale };

    if (size.isEmpty())
        return;

    bool committed { false };

    // Redrawn only when it may look different
    if (m_cursorRedraw || m_cursorImage != m_cursorDrawnImage || size != m_cursorDrawnSize)
    {
        m_cursorSwapchain->resize(size);
        auto image { m_cursorSwapchain->acquire() };

        if (!image)
            return;

        auto surface { RSurface::WrapImage(image->image) };
        auto pass { surface->beginPass(RPassCap_Painter) };
        auto *p { pass->getPainter() };
        p->save();
        RDrawImageInfo info {};
        info.image = m_cursorImage;
        info.dst = SkIRect::MakeSize(size);
        info.src = SkRect::Make(size);

        p->setBlendMode(RBlendMode::Src);
        p->drawImage(info);
        p->restore();
        pass.reset();

        wl_surface_set_buffer_scale(wl.cursorSurface, m_scale);
        m_cursorSwapchain->present(image.value());

        m_cursorRedraw = false;
        m_cursorDrawnImage = m_cursorImage;
        m_cursorDrawnSize = size;
        committed = true;

        // Presenting may have read the parent's events (e.g. Mesa waiting for a buffer): dispatched
        // from the main loop, not from here (this may run inside a dispatch)
        CZCore::Get()->postEvent(std::make_shared<CZPresentationEvent>(), *this);
    }

    if (committed || !m_cursorSent || serial != m_cursorSentSerial || m_cursorHotspot != m_cursorSentHotspot)
    {
        wl_pointer_set_cursor(wl.seat.pointer, serial, wl.cursorSurface, m_cursorHotspot.x(), m_cursorHotspot.y());
        m_cursorSent = true;
        m_cursorSentSerial = serial;
        m_cursorSentHotspot = m_cursorHotspot;
    }
}

bool LWaylandOutput::setCursor(UInt8 *pixels) noexcept
{
    if (!hasCursor())
        return false;

    if (pixels && cursor()->surface())
        m_cursorImage = cursor()->surface()->image();
    else
        m_cursorImage.reset();

    // A new image (or the same one with new content)
    m_cursorRedraw = true;
    updateCursor();
    return true;
}

LWaylandOutput::LWaylandOutput(LWaylandBackend *backend) noexcept : m_backend(backend) {}

bool LWaylandOutput::event(const CZEvent &e) noexcept
{
    if (e.type() == CZEvent::Type::Presentation)
    {
        // The scale changed on the render thread
        if (m_cursorScaleChanged.exchange(false))
            updateCursor();

        m_backend->dispatchEvents();
        return true;
    }

    return LBackendOutput::event(e);
}
