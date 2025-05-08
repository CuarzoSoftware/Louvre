#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LPointerMoveEvent.h>
#include <LPointerButtonEvent.h>
#include <LPointerScrollEvent.h>
#include <LKeyboardKeyEvent.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>
#include <LTouchFrameEvent.h>
#include <LTouchCancelEvent.h>
#include <LInputDevice.h>
#include <LUtils.h>
#include <LCursor.h>
#include <LLog.h>

#include <wayland-client.h>
#include <cstring>
#include <fcntl.h>

#include "../../graphic/Wayland/WaylandBackendShared.h"

#define BKND_NAME "WAYLAND BACKEND"

using namespace Louvre;

class Louvre::LInputBackend
{
public:

    static inline wl_display *display { nullptr };
    static inline wl_event_queue *queue;
    static inline wl_registry *registry;
    static inline wl_seat *seat { nullptr };
    static inline wl_pointer *pointer { nullptr };
    static inline wl_keyboard *keyboard { nullptr };
    static inline wl_touch *touch { nullptr };
    static inline LInputDevice device;
    static inline std::vector<LInputDevice*> devices;

    static inline wl_registry_listener registryListener;
    static inline wl_seat_listener seatListener;
    static inline wl_pointer_listener pointerListener;
    static inline wl_keyboard_listener keyboardListener;
    static inline wl_touch_listener touchListener;

    static inline wl_event_source *waylandEventSource { nullptr };
    static inline wl_event_source *eventfdEventSource { nullptr };

    static inline LPointerMoveEvent pointerMoveEvent;
    static inline LPointerButtonEvent pointerButtonEvent;
    static inline LPointerScrollEvent pointerScrollEvent;
    static inline LKeyboardKeyEvent keyboardKeyEvent;
    static inline LTouchDownEvent touchDownEvent;
    static inline LTouchUpEvent touchUpEvent;
    static inline LTouchMoveEvent touchMoveEvent;
    static inline LTouchFrameEvent touchFrameEvent;
    static inline LTouchCancelEvent touchCancelEvent;

    static inline UInt32 pointerEnterSerial;

    static WaylandBackendShared &shared()
    {
        return *static_cast<WaylandBackendShared*>( compositor()->imp()->graphicBackendData );
    }

    static void unlockRenderThread()
    {
        eventfd_write(shared().fd[0].fd, 1);
    }

    static UInt32 backendGetId()
    {
        return LInputBackendWayland;
    }

    static void *backendGetContextHandle()
    {
        return display;
    }

    static const std::vector<LInputDevice*> *backendGetDevices()
    {
        return &devices;
    }

    static void setupListeners()
    {
        registryListener.global         = registryHandleGlobal;
        registryListener.global_remove  = registryHandleGlobalRemove;
        seatListener.capabilities       = seatHandleCapabilities;
        seatListener.name               = seatHandleName;
        pointerListener.enter           = pointerHandleEnter;
        pointerListener.leave           = pointerHandleLeave;
        pointerListener.motion          = pointerHandleMotion;
        pointerListener.button          = pointerHandleButton;
        pointerListener.axis            = pointerHandleAxis;
        keyboardListener.keymap         = [](auto, auto, auto, auto, auto){};
        keyboardListener.enter          = keyboardHandleEnter;
        keyboardListener.leave          = keyboardHandleLeave;
        keyboardListener.key            = keyboardHandleKey;
        keyboardListener.modifiers      = [](auto, auto, auto, auto, auto, auto, auto){};
        touchListener.down              = touchHandleDown;
        touchListener.up                = touchHandleUp;
        touchListener.motion            = touchHandleMotion;
        touchListener.frame             = touchHandleFrame;
        touchListener.cancel            = touchHandleCancel;
    }

    static void setupEvents()
    {
        device.m_capabilities = 7;
        device.m_name = "Louvre Input Device";
        devices.push_back(&device);
        pointerMoveEvent.setDevice(&device);
        pointerButtonEvent.setDevice(&device);
        pointerScrollEvent.setDevice(&device);
        pointerScrollEvent.setSource(LPointerScrollEvent::Continuous);
        keyboardKeyEvent.setDevice(&device);
        touchDownEvent.setDevice(&device);
        touchUpEvent.setDevice(&device);
        touchMoveEvent.setDevice(&device);
        touchFrameEvent.setDevice(&device);
        touchCancelEvent.setDevice(&device);
    }

    static void updateCursor()
    {
        shared().mutex.lock();

        if ((!shared().cursorChangedBuffer && !shared().cursorChangedHotspot) || !pointer || !shared().cursorSurface)
        {
            shared().mutex.unlock();
            return;
        }

        if (shared().cursorChangedBuffer && shared().currentCursor)
        {
            wl_surface_damage(shared().cursorSurface, 0, 0, 512, 512);
            wl_surface_set_buffer_scale(shared().cursorSurface, 2);
            wl_surface_commit(shared().cursorSurface);
        }

        wl_pointer_set_cursor(pointer,
                              pointerEnterSerial,
                              shared().cursorVisible ? shared().cursorSurface : NULL,
                              shared().cursorHotspot.x(),
                              shared().cursorHotspot.y());

        shared().cursorChangedBuffer = false;
        shared().cursorChangedHotspot = false;

        shared().mutex.unlock();
    }

    static bool initWayland()
    {
        setupListeners();
        display = static_cast<wl_display*>(compositor()->graphicBackendContextHandle());
        queue = wl_display_create_queue(display);
        registry = wl_display_get_registry(display);
        wl_proxy_set_queue((wl_proxy*)registry, queue);
        wl_registry_add_listener(registry, &registryListener, nullptr);

        if (shared().cursorSurface)
            wl_proxy_set_queue((wl_proxy*)shared().cursorSurface, queue);

        wl_display_roundtrip_queue(display, queue);

        if (!seat)
        {
            LLog::error("[%s] Failed to get wl_seat v1.", BKND_NAME);
            return false;
        }

        shared().fd[2].fd = eventfd(0, O_CLOEXEC | O_NONBLOCK);

        eventfdEventSource = wl_event_loop_add_fd(
            LCompositor::eventLoop(),
            shared().fd[2].fd,
            WL_EVENT_READABLE,
            &LInputBackend::processInput,
            nullptr);

        waylandEventSource = wl_event_loop_add_fd(
            LCompositor::eventLoop(),
            shared().fd[1].fd,
            WL_EVENT_READABLE,
            &LInputBackend::processInput,
            nullptr);

        return true;
    }

    static void unitWayland()
    {
        if (eventfdEventSource)
        {
            wl_event_source_remove(eventfdEventSource);
            eventfdEventSource = nullptr;
            shared().fd[2].fd = -1;
        }

        if (waylandEventSource)
        {
            wl_event_source_remove(waylandEventSource);
            waylandEventSource = nullptr;
        }

        if (touch)
        {
            wl_touch_destroy(touch);
            touch = nullptr;
        }

        if (keyboard)
        {
            wl_keyboard_destroy(keyboard);
            keyboard = nullptr;
        }

        if (pointer)
        {
            wl_pointer_destroy(pointer);
            pointer = nullptr;
        }

        if (seat)
        {
            wl_seat_destroy(seat);
            seat = nullptr;
        }

        if (registry)
        {
            wl_registry_destroy(registry);
            registry = nullptr;
        }

        if (shared().cursorSurface)
            wl_proxy_set_queue((wl_proxy*)shared().cursorSurface, NULL);

        if (queue)
        {
            wl_event_queue_destroy(queue);
            queue = nullptr;
        }

        devices.clear();
        display = nullptr;
    }

    static bool backendInitialize()
    {
        if (compositor()->graphicBackendId() != LGraphicBackendWayland)
        {
            LLog::error("[%s] The Wayland input backend requires the Wayland graphic backend.", BKND_NAME);
            return false;
        }

        compositor()->imp()->inputBackendData = &shared();

        shared().mutex.lock();

        setupEvents();

        if (!initWayland())
        {
            shared().mutex.unlock();
            return false;
        }

        shared().mutex.unlock();

        return true;
    }

    static void backendUninitialize()
    {
        shared().mutex.lock();
        unitWayland();
        compositor()->imp()->inputBackendData = nullptr;
        shared().mutex.unlock();
    }

    static void backendSuspend(){}
    static void backendResume() {}

    static void backendForceUpdate()
    {
        processInput(-1, 0, nullptr);
    }

    static Int32 processInput(Int32 fd, UInt32 mask, void *)
    {
        if (fd == shared().fd[2].fd)
        {
            eventfd_t val;
            eventfd_read(fd, &val);
        }
        else
        {
            if (mask & (WL_EVENT_ERROR | WL_EVENT_HANGUP))
            {
                Louvre::compositor()->finish();
                return 0;
            }

            while (wl_display_prepare_read_queue(display, queue) != 0)
                wl_display_dispatch_queue_pending(display, queue);
            wl_display_flush(display);

            unlockRenderThread();

            pollfd waylandFd { shared().fd[1] };

            poll(&waylandFd, 1, 1);

            if (waylandFd.revents & POLLIN)
                wl_display_read_events(display);
            else
                wl_display_cancel_read(display);
        }

        updateCursor();
        wl_display_dispatch_queue_pending(display, queue);
        return 0;
    }

    static void registryHandleGlobal(void */*data*/, wl_registry *registry, uint32_t name, const char *interface, uint32_t /*version*/)
    {
        if (!seat && strcmp(interface, wl_seat_interface.name) == 0)
        {
            seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
            wl_proxy_set_queue((wl_proxy*)seat, queue);
            wl_seat_add_listener(seat, &seatListener, nullptr);
        }
    }

    static void registryHandleGlobalRemove(void */*data*/, struct wl_registry */*registry*/, uint32_t /*name*/) {}

    static void seatHandleCapabilities(void *, wl_seat *, UInt32 capabilities)
    {
        if (!pointer && capabilities & WL_SEAT_CAPABILITY_POINTER)
        {
            pointer = wl_seat_get_pointer(seat);
            wl_proxy_set_queue((wl_proxy*)pointer, queue);
            wl_pointer_add_listener(pointer, &pointerListener, nullptr);
        }

        if (!keyboard && capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
        {
            keyboard = wl_seat_get_keyboard(seat);
            wl_proxy_set_queue((wl_proxy*)keyboard, queue);
            wl_keyboard_add_listener(keyboard, &keyboardListener, nullptr);
        }

        if (!touch && capabilities & WL_SEAT_CAPABILITY_TOUCH)
        {
            touch = wl_seat_get_touch(seat);
            wl_proxy_set_queue((wl_proxy*)touch, queue);
            wl_touch_add_listener(touch, &touchListener, nullptr);
        }
    }

    static void seatHandleName(void *, wl_seat *, const char *) {}

    static void pointerHandleEnter(void *, wl_pointer *, UInt32 serial, wl_surface *, Float24 /*x*/, Float24 /*y*/)
    {
        pointerEnterSerial = serial;
        shared().cursorChangedBuffer = true;
    }

    static void pointerHandleLeave(void *, wl_pointer *, UInt32 /*serial*/, wl_surface *) {}

    static void pointerHandleMotion(void *, wl_pointer *, UInt32 time, Float24 x, Float24 y)
    {
        LPointF pos { (Float32)wl_fixed_to_double(x), (Float32)wl_fixed_to_double(y) };

        if (cursor()->output())
        {
            Float32 tmp;
            const LSizeF sizeF { shared().surfaceSize };
            switch (cursor()->output()->transform())
            {
            case LTransform::Normal:
                break;
            case LTransform::Rotated90:
                tmp = pos.y();
                pos.setY(pos.x());
                pos.setX(sizeF.h() - tmp);
                break;
            case LTransform::Rotated180:
                pos.setY(sizeF.h() - pos.y());
                pos.setX(sizeF.w() - pos.x());
                break;
            case LTransform::Rotated270:
                tmp = pos.y();
                pos.setY(sizeF.w() - pos.x());
                pos.setX(tmp);
                break;
            case LTransform::Flipped:
                pos.setX(sizeF.w() - pos.x());
                break;
            case LTransform::Flipped90:
                tmp = pos.y();
                pos.setY(pos.x());
                pos.setX(tmp);
                break;
            case LTransform::Flipped180:
                pos.setY(sizeF.h() - pos.y());
                break;
            case LTransform::Flipped270:
                tmp = pos.y();
                pos.setY(sizeF.w() - pos.x());
                pos.setX(sizeF.h() - tmp);
                break;
            }
        }

        pointerMoveEvent.setDelta(pos - cursor()->pos());
        pointerMoveEvent.setDeltaUnaccelerated(pointerMoveEvent.delta());
        pointerMoveEvent.setSerial(LTime::nextSerial());
        pointerMoveEvent.setMs(time);
        pointerMoveEvent.setUs(LTime::us());
        pointerMoveEvent.notify();
    }

    static void pointerHandleButton(void *, wl_pointer *, UInt32 serial, UInt32 time, UInt32 button, UInt32 state)
    {
        pointerButtonEvent.setButton((LPointerButtonEvent::Button)button);
        pointerButtonEvent.setState((LPointerButtonEvent::State)state);
        pointerButtonEvent.setSerial(serial);
        pointerButtonEvent.setMs(time);
        pointerButtonEvent.setUs(LTime::us());
        pointerButtonEvent.notify();
    }

    static void pointerHandleAxis(void *, wl_pointer *, UInt32 time, UInt32 axis, Float24 value)
    {
        const Float64 val { wl_fixed_to_double(value) };

        // This would be interpreted as a stop event
        if (val == 0.f)
            return;

        pointerScrollEvent.setSerial(LTime::nextSerial());
        pointerScrollEvent.setMs(time);
        pointerScrollEvent.setUs(LTime::us());

        pointerScrollEvent.setHasX(axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL);
        pointerScrollEvent.setHasY(axis == WL_POINTER_AXIS_VERTICAL_SCROLL);

        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
        {
            pointerScrollEvent.setX(val);
            pointerScrollEvent.setY(0.f);
        }
        else
        {
            pointerScrollEvent.setY(val);
            pointerScrollEvent.setX(0.f);
        }

        pointerScrollEvent.notify();
    }

    static void keyboardHandleEnter(void *data, wl_keyboard *keyboard, UInt32 /*serial*/, wl_surface *, wl_array *keysArr)
    {
        if (keysArr->size == 0)
            return;

        UInt32 *key { static_cast<UInt32*>(keysArr->data)};
        std::size_t n { keysArr->size/sizeof(*key) };
        while (n > 0)
        {
            keyboardHandleKey(data, keyboard, LTime::nextSerial(), LTime::ms(), *key, WL_KEYBOARD_KEY_STATE_PRESSED);
            n--;
            key++;
        }
    }

    static void keyboardHandleLeave(void *, wl_keyboard*, UInt32 /*serial*/, wl_surface *)
    {
        while (!Louvre::seat()->keyboard()->pressedKeys().empty())
            keyboardHandleKey(nullptr, nullptr, LTime::nextSerial(), LTime::ms(), Louvre::seat()->keyboard()->pressedKeys().back(), WL_KEYBOARD_KEY_STATE_RELEASED);
    }

    static void keyboardHandleKey(void *, wl_keyboard*, UInt32 serial, UInt32 time, UInt32 key, UInt32 state)
    {
        keyboardKeyEvent.setState((LKeyboardKeyEvent::State)state);
        keyboardKeyEvent.setKeyCode(key);
        keyboardKeyEvent.setSerial(serial);
        keyboardKeyEvent.setMs(time);
        keyboardKeyEvent.setUs(LTime::us());
        keyboardKeyEvent.notify();
    }

    static LPointF normalizedTouchPoint(Float24 x, Float24 y)
    {
        LPointF point((Float32)wl_fixed_to_double(x), (Float32) wl_fixed_to_double(y));

        if (point.x() < 0.f)
            point.setX(0.f);

        if (point.y() < 0.f)
            point.setY(0.f);

        if (point.x() > shared().surfaceSize.w())
            point.setX(shared().surfaceSize.w());

        if (point.y() > shared().surfaceSize.h())
            point.setY(shared().surfaceSize.h());

        if (shared().surfaceSize.w() != 0)
            point.setX(point.x()/Float64(shared().surfaceSize.w()));

        if (shared().surfaceSize.h() != 0)
            point.setY(point.y()/Float64(shared().surfaceSize.h()));

        return point;
    }

    static void touchHandleDown(void *, wl_touch *, UInt32 serial, UInt32 time, wl_surface*, Int32 id, Float24 x, Float24 y)
    {
        const LPointF normalizedPoint { normalizedTouchPoint(x, y) };
        touchDownEvent.setId(id);
        touchDownEvent.setSerial(serial);
        touchDownEvent.setMs(time);
        touchDownEvent.setUs(LTime::us());
        touchDownEvent.setX(normalizedPoint.x());
        touchDownEvent.setY(normalizedPoint.y());
        touchDownEvent.notify();
    }

    static void touchHandleUp(void *, wl_touch *, UInt32 serial, UInt32 time, Int32 id)
    {
        touchUpEvent.setId(id);
        touchUpEvent.setSerial(serial);
        touchUpEvent.setMs(time);
        touchUpEvent.setUs(LTime::us());
        touchUpEvent.notify();
    }

    static void touchHandleMotion(void *, wl_touch *, UInt32 time, Int32 id, Float24 x, Float24 y)
    {
        const LPointF normalizedPoint { normalizedTouchPoint(x, y) };
        touchMoveEvent.setId(id);
        touchMoveEvent.setSerial(LTime::nextSerial());
        touchMoveEvent.setMs(time);
        touchMoveEvent.setUs(LTime::us());
        touchMoveEvent.setX(normalizedPoint.x());
        touchMoveEvent.setY(normalizedPoint.y());
        touchMoveEvent.notify();
    }

    static void touchHandleFrame(void *, wl_touch *)
    {
        touchFrameEvent.setSerial(LTime::nextSerial());
        touchFrameEvent.setMs(LTime::ms());
        touchFrameEvent.setUs(LTime::us());
        touchFrameEvent.notify();
    }

    static void touchHandleCancel(void *, wl_touch *)
    {
        touchCancelEvent.setSerial(LTime::nextSerial());
        touchCancelEvent.setMs(LTime::ms());
        touchCancelEvent.setUs(LTime::us());
        touchCancelEvent.notify();
    }
};

extern "C" LInputBackendInterface *getAPI()
{
    static LInputBackendInterface API;
    API.backendGetId            = &LInputBackend::backendGetId;
    API.backendGetContextHandle = &LInputBackend::backendGetContextHandle;
    API.backendGetDevices       = &LInputBackend::backendGetDevices;
    API.backendInitialize       = &LInputBackend::backendInitialize;
    API.backendUninitialize     = &LInputBackend::backendUninitialize;
    API.backendSuspend          = &LInputBackend::backendSuspend;
    API.backendResume           = &LInputBackend::backendResume;
    API.backendSetLeds          = NULL;
    API.backendForceUpdate      = &LInputBackend::backendForceUpdate;
    return &API;
}
