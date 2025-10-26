#include <CZ/Core/Events/CZPointerScrollEvent.h>
#include <CZ/Core/Events/CZTouchCancelEvent.h>
#include <CZ/Core/Events/CZTouchDownEvent.h>
#include <CZ/Core/Events/CZTouchFrameEvent.h>
#include <CZ/Core/Events/CZTouchMoveEvent.h>
#include <CZ/Core/Events/CZTouchUpEvent.h>
#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>

#include <CZ/Core/CZEventSource.h>
#include <CZ/Core/CZKeymap.h>
#include <CZ/Core/CZCore.h>

#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/WL/RWLPlatformHandle.h>
#include <CZ/Ream/RCore.h>

#include <CZ/Louvre/Backends/Wayland/LWaylandOutput.h>
#include <CZ/Louvre/Backends/Wayland/LWaylandBackend.h>
#include <CZ/Louvre/Private/LLockGuard.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/LDMAFeedback.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LKeyboard.h>

#include <sys/poll.h>
#include <wayland-client-core.h>

using namespace CZ;

static const wl_pointer_listener pointerLis
{
    .enter = [](void *data, wl_pointer */*pointer*/, UInt32 serial, wl_surface */*surface*/, wl_fixed_t x, wl_fixed_t y)
    {
        auto &wl { static_cast<LWaylandBackend*>(data)->wl };
        wl.seat.pointerEnterSerial = serial;
        auto diff { SkPoint(wl_fixed_to_double(x), wl_fixed_to_double(y)) - cursor()->pos() };
        CZPointerMoveEvent e {};
        e.delta = e.deltaUnaccelerated = diff;
        CZCore::Get()->sendEvent(e, *seat());
    },
    .leave = [](void */*data*/, wl_pointer */*pointer*/, UInt32 /*serial*/, wl_surface */*surface*/)
    {
        CZPointerButtonEvent e {};
        e.pressed = false;
        while (!seat()->pointer()->pressedButtons.empty())
        {
            e.serial = CZTime::NextSerial();
            e.button = *seat()->pointer()->pressedButtons.begin();
            CZCore::Get()->sendEvent(e, *seat());
        }
    },
    .motion = [](void */*data*/, wl_pointer */*pointer*/, UInt32 /*time*/, wl_fixed_t x, wl_fixed_t y)
    {
        auto diff { SkPoint(wl_fixed_to_double(x), wl_fixed_to_double(y)) - cursor()->pos() };
        CZPointerMoveEvent e {};
        e.delta = e.deltaUnaccelerated = diff;
        CZCore::Get()->sendEvent(e, *seat());
    },
    .button = [](void */*data*/, wl_pointer */*pointer*/, UInt32 /*serial*/, UInt32 /*time*/, UInt32 button, UInt32 state)
    {
        CZPointerButtonEvent e {};
        e.button = button;
        e.pressed = state;
        CZCore::Get()->sendEvent(e, *seat());
    },
    .axis = [](void */*data*/, wl_pointer */*pointer*/, UInt32 /*time*/, UInt32 axis, wl_fixed_t value)
    {
        CZPointerScrollEvent e {};
        e.source = CZPointerScrollEvent::Source::Continuous;

        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
        {
            e.hasX = true;
            e.axes.fX = wl_fixed_to_double(value);
        }
        else
        {
            e.hasY = true;
            e.axes.fY = wl_fixed_to_double(value);
        }
        CZCore::Get()->sendEvent(e, *seat());
    },
    .frame = {},
    .axis_source = {},
    .axis_stop = {},
    .axis_discrete = {},
    .axis_value120 = {},
    .axis_relative_direction = {}
};

static const wl_keyboard_listener keyboardLis
{
    .keymap = [](auto, auto, auto, Int32 fd, auto) { close(fd); },
    .enter = [](auto, auto, auto, auto, wl_array *keys)
    {
        CZKeyboardKeyEvent e {};
        e.isPressed = true;

        auto *k { (UInt32*)keys->data };
        for (size_t i = 0; i < keys->size/sizeof(UInt32); i++)
        {
            e.code = k[i];
            CZCore::Get()->sendEvent(e, *seat());
            e.serial = CZTime::NextSerial();
        }
    },

    .leave = [](auto, auto, auto, auto)
    {
        CZKeyboardKeyEvent e {};
        e.isPressed = false;

        while (!seat()->keyboard()->keymap()->pressedKeys().empty())
        {
            e.code = *seat()->keyboard()->keymap()->pressedKeys().begin();
            CZCore::Get()->sendEvent(e, *seat());
        }
    },

    .key = [](auto, auto, auto, UInt32 time, UInt32 key, UInt32 state)
    {
        CZKeyboardKeyEvent e {};
        e.isPressed = state;
        e.code = key;
        e.ms = time;
        CZCore::Get()->sendEvent(e, *seat());
    },
    .modifiers = [](auto, auto, auto, auto, auto, auto, auto){},
    .repeat_info = {}
};

static const wl_touch_listener touchLis
{
    .down = [](void *data, auto, auto, auto, auto, Int32 id, wl_fixed_t x, wl_fixed_t y)
    {
        auto *backend { static_cast<LWaylandBackend*>(data) };
        CZTouchDownEvent e {};
        e.id = id;
        e.pos.fX = wl_fixed_to_double(x)/Float32(backend->outputs()[0]->size().width());
        e.pos.fY = wl_fixed_to_double(y)/Float32(backend->outputs()[0]->size().height());
        CZCore::Get()->sendEvent(e, *seat());
    },
    .up = [](auto, auto, auto, auto, Int32 id)
    {
        CZTouchUpEvent e {};
        e.id = id;
        CZCore::Get()->sendEvent(e, *seat());
    },
    .motion = [](void *data, auto, auto, Int32 id, wl_fixed_t x, wl_fixed_t y)
    {
        auto *backend { static_cast<LWaylandBackend*>(data) };
        CZTouchMoveEvent e {};
        e.id = id;
        e.pos.fX = wl_fixed_to_double(x)/Float32(backend->outputs()[0]->size().width());
        e.pos.fY = wl_fixed_to_double(y)/Float32(backend->outputs()[0]->size().height());
        CZCore::Get()->sendEvent(e, *seat());
    },

    .frame = [](auto, auto)
    {
        CZTouchFrameEvent e {};
        CZCore::Get()->sendEvent(e, *seat());
    },
    .cancel = [](auto, auto)
    {
        CZTouchCancelEvent e {};
        CZCore::Get()->sendEvent(e, *seat());
    },
    .shape = {},
    .orientation = {}
};

static const wl_seat_listener seatLis
{
    .capabilities = [](void *data, wl_seat *seat, UInt32 caps)
    {
        auto &wl { static_cast<LWaylandBackend*>(data)->wl };

        if (caps & WL_SEAT_CAPABILITY_POINTER)
        {
            if (!wl.seat.pointer)
            {
                wl.seat.pointer = wl_seat_get_pointer(seat);
                wl_pointer_add_listener(wl.seat.pointer, &pointerLis, data);
            }
        }
        else
        {
            if (wl.seat.pointer)
            {
                pointerLis.leave(nullptr, nullptr, 0, nullptr);
                wl_pointer_destroy(wl.seat.pointer);
                wl.seat.pointer = nullptr;
            }
        }

        if (caps & WL_SEAT_CAPABILITY_KEYBOARD)
        {
            if (!wl.seat.keyboard)
            {
                wl.seat.keyboard = wl_seat_get_keyboard(seat);
                wl_keyboard_add_listener(wl.seat.keyboard, &keyboardLis, data);
            }
        }
        else
        {
            if (wl.seat.keyboard)
            {
                keyboardLis.leave(nullptr, nullptr, 0, nullptr);
                wl_keyboard_destroy(wl.seat.keyboard);
                wl.seat.keyboard = nullptr;
            }
        }

        if (caps & WL_SEAT_CAPABILITY_TOUCH)
        {
            if (!wl.seat.touch)
            {
                wl.seat.touch = wl_seat_get_touch(seat);
                wl_touch_add_listener(wl.seat.touch, &touchLis, data);
            }
        }
        else
        {
            if (wl.seat.touch)
            {
                touchLis.cancel(nullptr, nullptr);
                wl_touch_destroy(wl.seat.touch);
                wl.seat.touch = nullptr;
            }
        }
    },
    .name = {}
};

static const xdg_wm_base_listener xdgWmBaseLis
{
    .ping = [](void */*data*/, xdg_wm_base *xdgWmBase, UInt32 serial)
    {
        xdg_wm_base_pong(xdgWmBase, serial);
    }
};

static const wl_registry_listener registryLis
{
    .global = [](void *data, wl_registry *registry, UInt32 name, const char *interface, UInt32 version)
    {
        auto &wl { static_cast<LWaylandBackend*>(data)->wl };

        if (!wl.compositor && strcmp(interface, wl_compositor_interface.name) == 0 && version >= 6)
        {
            wl.compositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 6);
        }
        else if (!wl.xdgWmBase && strcmp(interface, xdg_wm_base_interface.name) == 0)
        {
            wl.xdgWmBase = (xdg_wm_base*)wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
            xdg_wm_base_add_listener(wl.xdgWmBase, &xdgWmBaseLis, data);
        }
        else if (!wl.seat.proxy && strcmp(interface, wl_seat_interface.name) == 0)
        {
            wl.seat.proxy = (wl_seat*)wl_registry_bind(registry, name, &wl_seat_interface, 1);
            wl_seat_add_listener(wl.seat.proxy, &seatLis, data);
        }
    },
    .global_remove = [](auto, auto, auto) {}
};

bool LWaylandBackend::init() noexcept
{
    const auto ret {
        initDisplay() &&
        initReam() &&
        initOutput() };

    if (!ret)
        unit();

    return ret;
}

void LWaylandBackend::unit() noexcept
{
    while (!m_outputs.empty())
    {
        seat()->imp()->handleOutputUnplugged(m_outputs.back());
        delete m_outputs.back();
        m_outputs.pop_back();
    }

    m_defaultFeedback.reset();

    if (wl.xdgWmBase)
    {
        xdg_wm_base_destroy(wl.xdgWmBase);
        wl.xdgWmBase = {};
    }

    if (wl.compositor)
    {
        wl_compositor_destroy(wl.compositor);
        wl.compositor = {};
    }

    if (wl.registry)
    {
        wl_registry_destroy(wl.registry);
        wl.registry = {};
    }

    m_source.reset();
    m_ream.reset();
    wl.display = {};
}

bool LWaylandBackend::initDisplay() noexcept
{
    wl.display = wl_display_connect(NULL);

    if (!wl.display)
    {
        log(CZFatal, CZLN, "wl_display_connect failed");
        return false;
    }

    m_source = CZEventSource::Make(
        wl_display_get_fd(wl.display),
        EPOLLIN,
        CZOwn::Borrow,
        [this](int fd, UInt32 events){

            pollfd pfd {};
            pfd.fd = fd;
            pfd.events = events;
            if (poll(&pfd, 1, 0) > 0)
                wl_display_dispatch(wl.display);
        });

    wl.registry = wl_display_get_registry(wl.display);
    wl_registry_add_listener(wl.registry, &registryLis, this);
    wl_display_roundtrip(wl.display);

    if (!wl.compositor)
    {
        log(CZFatal, CZLN, "wl_compositor >= 6 unsupported by the parent compositor");
        return false;
    }

    if (!wl.xdgWmBase)
    {
        log(CZFatal, CZLN, "xdg_wm_base unsupported by the parent compositor");
        return false;
    }

    return true;
}

bool LWaylandBackend::initReam() noexcept
{
    RCore::Options options {};
    options.platformHandle = RWLPlatformHandle::Make(wl.display, CZOwn::Own);
    options.graphicsAPI = RGraphicsAPI::Auto;
    m_ream = RCore::Make(options);

    if (m_ream)
        initDefaultFeedback();
    else
        log(CZFatal, CZLN, "Failed to initialize RCore");

    return m_ream != nullptr;
}

bool LWaylandBackend::initOutput() noexcept
{
    m_outputs.emplace_back(LWaylandOutput::Make(this));
    return m_outputs[0] != nullptr;
}

void LWaylandBackend::initDefaultFeedback() noexcept
{
    auto ream { RCore::Get() };
    auto supportedFormats { RDRMFormatSet::Intersect(ream->mainDevice()->dmaTextureFormats(), ream->mainDevice()->textureFormats()) };

    if (ream->mainDevice()->id() == 0 || supportedFormats.formats().empty())
    {
        log(CZError, CZLN, "Failed to create default DMA feedback");
        return;
    }

    std::vector<LDMAFeedback::Tranche> tranches;
    tranches.reserve(4);

    // Main device texture tranche
    LDMAFeedback::Tranche tranche {};
    tranche.device = ream->mainDevice();
    tranche.flags = 0;
    tranche.formatSet = supportedFormats;
    tranches.emplace_back(std::move(tranche));
    m_defaultFeedback = LDMAFeedback::Make(ream->mainDevice(), std::move(tranches));
}
