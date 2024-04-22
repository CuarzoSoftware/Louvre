#include <wayland-client.h>
#include <cstring>
#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LInputDevice.h>
#include <LPointerMoveEvent.h>
#include <LPointerButtonEvent.h>
#include <LPointerScrollEvent.h>
#include <LPointerSwipeBeginEvent.h>
#include <LPointerSwipeUpdateEvent.h>
#include <LPointerSwipeEndEvent.h>
#include <LPointerPinchBeginEvent.h>
#include <LPointerPinchUpdateEvent.h>
#include <LPointerPinchEndEvent.h>
#include <LPointerHoldBeginEvent.h>
#include <LPointerHoldEndEvent.h>
#include <LKeyboardKeyEvent.h>
#include <LTouchDownEvent.h>
#include <LTouchMoveEvent.h>
#include <LTouchUpEvent.h>
#include <LTouchFrameEvent.h>
#include <LTouchCancelEvent.h>
#include <LUtils.h>
#include <LCursor.h>
#include <LLog.h>


#include <libinput.h>
#include <fcntl.h>

#define BKND_NAME "WAYLAND BACKEND"

using namespace Louvre;

struct WaylandBackendShared
{
    LSize surfaceSize { 1024, 512 };
    LSize bufferSize { 1024, 512 };
    Int32 bufferScale { 1 };
};

struct Backend
{
    wl_display *display { nullptr };
    wl_registry *registry;
    wl_seat *seat { nullptr };
    wl_pointer *pointer { nullptr };
    LInputDevice device;
    std::vector<LInputDevice*> devices;
    LPointerMoveEvent pointerMoveEvent;
    LPointerButtonEvent pointerButtonEvent;
    LPointerScrollEvent pointerScrollEvent;
    LKeyboardKeyEvent keyboardKeyEvent;

    wl_registry_listener registryListener;
    wl_seat_listener seatListener;
    wl_pointer_listener pointerListener;

    wl_event_source *eventSource { nullptr };
};

class Louvre::LInputBackend
{
public:

    static Backend &backend()
    {
        return *static_cast<Backend*>( seat()->imp()->inputBackendData );
    }

    static WaylandBackendShared &shared()
    {
        return *static_cast<WaylandBackendShared*>( compositor()->imp()->graphicBackendData );
    }

    static UInt32 backendGetId()
    {
        return LInputBackendWayland;
    }

    static UInt32 backendGetCapabilities()
    {
        return LSeat::InputCapabilities::Pointer | LSeat::InputCapabilities::Keyboard | LSeat::InputCapabilities::Touch;
    }

    static void *backendGetContextHandle()
    {
        return backend().display;
    }

    static const std::vector<LInputDevice*> *backendGetDevices()
    {
        return &backend().devices;
    }

    static void setupListeners()
    {
        backend().registryListener.global        = registryHandleGlobal;
        backend().registryListener.global_remove = registryHandleGlobalRemove;
        backend().seatListener.capabilities      = seatHandleCapabilities;
        backend().seatListener.name              = seatHandleName;
        backend().pointerListener.enter          = pointerHandleEnter;
        backend().pointerListener.leave          = pointerHandleLeave;
        backend().pointerListener.motion         = pointerHandleMotion;
        backend().pointerListener.button         = pointerHandleButton;
        backend().pointerListener.axis           = pointerHandleAxis;
    }

    static void setupEvents()
    {
        backend().device.m_capabilities = LSeat::InputCapabilities::Pointer | LSeat::InputCapabilities::Keyboard | LSeat::InputCapabilities::Touch;
        backend().device.m_name = "Louvre Input Device";
        backend().devices.push_back(&backend().device);
        backend().pointerMoveEvent.setDevice(&backend().device);
        backend().pointerButtonEvent.setDevice(&backend().device);
        backend().pointerScrollEvent.setDevice(&backend().device);
        backend().pointerScrollEvent.setSource(LPointerScrollEvent::Wheel);
        backend().keyboardKeyEvent.setDevice(&backend().device);
    }

    static bool initWayland()
    {
        setupListeners();
        backend().display = static_cast<wl_display*>(seat()->graphicBackendContextHandle());
        backend().registry = wl_display_get_registry(backend().display);
        wl_registry_add_listener(backend().registry, &backend().registryListener, nullptr);
        wl_display_roundtrip(backend().display);

        if (!backend().seat)
        {
            LLog::error("[%s] Failed to get wl_seat v1.", BKND_NAME);
            return false;
        }

        backend().eventSource = compositor()->addFdListener(
            wl_display_get_fd(backend().display),
            nullptr,
            LInputBackend::processInput,
            WL_EVENT_READABLE);

        return true;
    }

    static bool backendInitialize()
    {
        if (seat()->graphicBackendId() != LGraphicBackendWayland)
        {
            LLog::error("[%s] The Wayland input backend requires the Wayland graphic backend.", BKND_NAME);
            return false;
        }

        Backend *backend { new Backend() };
        seat()->imp()->inputBackendData = backend;
        setupEvents();

        if (!initWayland())
        {
            delete backend;
            return false;
        }

        return true;
    }

    static void backendUninitialize()
    {
        if (backend().eventSource)
            compositor()->removeFdListener(backend().eventSource);

        delete &backend();
    }

    static void backendSuspend()
    {
        if (backend().eventSource)
        {
            compositor()->removeFdListener(backend().eventSource);
            backend().eventSource = nullptr;
        }
    }

    static void backendResume()
    {
        if (!backend().eventSource)
        {
            backend().eventSource = compositor()->addFdListener(
                wl_display_get_fd(backend().display),
                nullptr,
                LInputBackend::processInput,
                WL_EVENT_READABLE | WL_EVENT_WRITABLE);
        }
    }

    static void backendForceUpdate()
    {
        processInput(0, WL_EVENT_READABLE | WL_EVENT_WRITABLE, nullptr);
    }

    static Int32 processInput(Int32, UInt32 mask, void *)
    {
        if (mask & WL_EVENT_READABLE)
            wl_display_dispatch(backend().display);

        if (mask & WL_EVENT_WRITABLE)
            wl_display_flush(backend().display);

        return 0;
    }

    static void registryHandleGlobal(void */*data*/, wl_registry *registry, uint32_t name, const char *interface, uint32_t /*version*/)
    {
        if (!backend().seat && strcmp(interface, wl_seat_interface.name) == 0)
        {
            backend().seat = static_cast<wl_seat*>(wl_registry_bind(registry, name, &wl_seat_interface, 1));
            wl_seat_add_listener(backend().seat, &backend().seatListener, nullptr);
        }
    }

    static void registryHandleGlobalRemove(void */*data*/, struct wl_registry */*registry*/, uint32_t /*name*/)
    {
        // TODO
    }

    static void seatHandleCapabilities(void *, wl_seat *, UInt32 capabilities)
    {
        if (!backend().pointer && capabilities & LSeat::InputCapabilities::Pointer)
        {
            backend().pointer = wl_seat_get_pointer(backend().seat);
            wl_pointer_add_listener(backend().pointer, &backend().pointerListener, nullptr);
        }
    }

    static void seatHandleName(void *, wl_seat *, const char *) {}

    static void pointerHandleEnter(void *, wl_pointer *, UInt32 serial, wl_surface *, Float24 /*x*/, Float24 /*y*/)
    {
        wl_pointer_set_cursor(backend().pointer, serial, NULL, 0, 0);
    }

    static void pointerHandleLeave(void *, wl_pointer *, UInt32 /*serial*/, wl_surface *) {}

    static void pointerHandleMotion(void *, wl_pointer *, UInt32 time, Float24 x, Float24 y)
    {
        LPointF delta { (Float32)wl_fixed_to_double(x), (Float32)wl_fixed_to_double(y) };
        //delta *= (Float32)shared().bufferScale;
        backend().pointerMoveEvent.setDelta(delta - cursor()->pos());
        backend().pointerMoveEvent.setDeltaUnaccelerated(backend().pointerMoveEvent.delta());
        backend().pointerMoveEvent.setSerial(LTime::nextSerial());
        backend().pointerMoveEvent.setMs(time);
        backend().pointerMoveEvent.setUs(LTime::us());
        backend().pointerMoveEvent.notify();
    }

    static void pointerHandleButton(void *, wl_pointer *, UInt32 serial, UInt32 time, UInt32 button, UInt32 state)
    {
        backend().pointerButtonEvent.setButton((LPointerButtonEvent::Button)button);
        backend().pointerButtonEvent.setState((LPointerButtonEvent::State)state);
        backend().pointerButtonEvent.setSerial(serial);
        backend().pointerButtonEvent.setMs(time);
        backend().pointerButtonEvent.setUs(LTime::us());
        backend().pointerButtonEvent.notify();
    }

    static void pointerHandleAxis(void *, wl_pointer *, UInt32 time, UInt32 axis, Float24 value)
    {
        Float32 val = wl_fixed_to_double(value);
        backend().pointerScrollEvent.setSerial(LTime::nextSerial());
        backend().pointerScrollEvent.setMs(time);
        backend().pointerScrollEvent.setUs(LTime::us());

        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
            backend().pointerScrollEvent.setX(val);
        else
            backend().pointerScrollEvent.setY(val);

        backend().pointerScrollEvent.notify();
    }

};

extern "C" LInputBackendInterface *getAPI()
{
    static LInputBackendInterface API;
    API.backendGetId            = &LInputBackend::backendGetId;
    API.backendGetCapabilities  = &LInputBackend::backendGetCapabilities;
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
