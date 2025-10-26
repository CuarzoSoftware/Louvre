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

using namespace CZ;

static wl_surface_listener surfaceLis {};
static xdg_surface_listener xdgSurfaceLis {};
static xdg_toplevel_listener xdgToplevelLis {};
static wl_callback_listener callbackLis {};

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

    o->output()->imp()->backendPresented(timeInfo);

    wl_callback_destroy(callback);
    o->m_backend->wl.callback = nullptr;

    if (o->m_unitPromise.has_value())
        return;

    if (o->m_pendingRepaint)
        o->repaint();
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

    wl.cursorSurface = wl_compositor_create_surface(wl.compositor);
    o->m_cursorSwapchain = RWLSwapchain::Make(wl.cursorSurface, {64, 64});

    LOutput::Params params {};
    params.backend.reset(o);
    return LFactory::createObject<LOutput>(&params);
}

LWaylandOutput::~LWaylandOutput() noexcept
{
    auto &wl { m_backend->wl };

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
            m_semaphore.acquire();

            LLockGuard lock {};

            if (m_unitPromise.has_value())
            {
                m_unitPromise->set_value(true);
                break;
            }

            if (wl.callback || !m_configured)
                continue;

            m_paintEventId++;

            const SkISize newSize { m_size.fWidth * m_scale, m_size.fHeight * m_scale };

            wl_surface_set_buffer_scale(wl.surface, m_scale);

            if (!m_swapchain)
                m_swapchain = RWLSwapchain::Make(wl.surface, newSize);

            if (output()->transform() != CZTransform::Normal || newSize != mode->m_size)
            {
                output()->setTransform(CZTransform::Normal);
                mode->m_size = newSize;
                m_swapchain->resize(newSize);
                output()->imp()->backendResizeGL();
                updateCursor();
            }

            m_pendingRepaint = false;

            auto image { m_swapchain->acquire() };
            m_images[0] = image->image;
            output()->setScale(m_scale);
            m_age = image->age;
            output()->imp()->backendPaintGL();
            wl.callback = wl_surface_frame(wl.surface);
            wl_callback_add_listener(wl.callback, &callbackLis, this);
            m_swapchain->present(image.value(), m_damage.has_value() ? &m_damage.value() : nullptr);
            m_damage.reset();
            core->postEvent(event, *this);
        }

        output()->imp()->backendUninitializeGL();
    }).detach();

    return true;
}

bool LWaylandOutput::repaint() noexcept
{
    m_pendingRepaint = true;
    m_semaphore.release();
    return true;
}

void LWaylandOutput::unit() noexcept
{
    m_unitPromise = std::promise<bool>();
    auto future { m_unitPromise->get_future() };
    m_semaphore.release();
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

    auto src { cursor()->source() };

    m_cursorHotspot = SkIPoint(
        (src->hotspot().x() * cursor()->size().width())/SkScalar(src->image()->size().width()),
        (src->hotspot().y() * cursor()->size().height())/SkScalar(src->image()->size().height())
    );

    updateCursor();
    return true;
}

void LWaylandOutput::updateCursor() noexcept
{
    if (!m_cursorSwapchain)
        return;

    if (m_cursorImage)
    {
        const SkISize size { cursor()->rect().width() * m_scale, cursor()->rect().height() * m_scale };
        m_cursorSwapchain->resize(size);
        auto image { m_cursorSwapchain->acquire() };
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

        wl_surface_set_buffer_scale(m_backend->wl.cursorSurface, m_scale);
        m_cursorSwapchain->present(image.value());
        wl_pointer_set_cursor(m_backend->wl.seat.pointer,
                              m_backend->wl.seat.pointerEnterSerial,
                              m_backend->wl.cursorSurface,
                              m_cursorHotspot.x(), m_cursorHotspot.y());
    }
    else
    {
        wl_pointer_set_cursor(m_backend->wl.seat.pointer, m_backend->wl.seat.pointerEnterSerial, NULL, 0, 0);
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

    updateCursor();
    return true;
}

LWaylandOutput::LWaylandOutput(LWaylandBackend *backend) noexcept : m_backend(backend) {}

bool LWaylandOutput::event(const CZEvent &e) noexcept
{
    if (e.type() == CZEvent::Type::Presentation)
    {
        wl_display_flush(m_backend->wl.display);
        return true;
    }

    return LBackendOutput::event(e);
}
