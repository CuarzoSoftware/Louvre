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
    std::mutex mutex;
    pollfd fd[3];
    LSize surfaceSize { 1024, 512 };
    LSize bufferSize { 1024, 512 };
    Int32 bufferScale { 1 };
    wl_buffer *cursorBuffer { nullptr };
    wl_surface *cursorSurface { nullptr };
    bool cursorChangedHotspot { false };
    bool cursorChangedBuffer { false };
    bool cursorVisible { false };
    LPoint cursorHotspot;
};

class Louvre::LInputBackend
{
public:

    static inline wl_display *display { nullptr };
    static inline wl_event_queue *queue;
    static inline wl_registry *registry;
    static inline wl_seat *seat { nullptr };
    static inline wl_pointer *pointer { nullptr };
    static inline LInputDevice device;
    static inline std::vector<LInputDevice*> devices;

    static inline wl_registry_listener registryListener;
    static inline wl_seat_listener seatListener;
    static inline wl_pointer_listener pointerListener;

    static inline wl_event_source *eventSource { nullptr };

    static inline LPointerMoveEvent pointerMoveEvent;
    static inline LPointerButtonEvent pointerButtonEvent;
    static inline LPointerScrollEvent pointerScrollEvent;
    static inline LKeyboardKeyEvent keyboardKeyEvent;

    static inline UInt32 enterSerial;

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

    static UInt32 backendGetCapabilities()
    {
        return LSeat::InputCapabilities::Pointer | LSeat::InputCapabilities::Keyboard | LSeat::InputCapabilities::Touch;
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
        registryListener.global        = registryHandleGlobal;
        registryListener.global_remove = registryHandleGlobalRemove;
        seatListener.capabilities      = seatHandleCapabilities;
        seatListener.name              = seatHandleName;
        pointerListener.enter          = pointerHandleEnter;
        pointerListener.leave          = pointerHandleLeave;
        pointerListener.motion         = pointerHandleMotion;
        pointerListener.button         = pointerHandleButton;
        pointerListener.axis           = pointerHandleAxis;
    }

    static void setupEvents()
    {
        device.m_capabilities = LSeat::InputCapabilities::Pointer | LSeat::InputCapabilities::Keyboard | LSeat::InputCapabilities::Touch;
        device.m_name = "Louvre Input Device";
        devices.push_back(&device);
        pointerMoveEvent.setDevice(&device);
        pointerButtonEvent.setDevice(&device);
        pointerScrollEvent.setDevice(&device);
        pointerScrollEvent.setSource(LPointerScrollEvent::Wheel);
        keyboardKeyEvent.setDevice(&device);
    }

    static void updateCursor()
    {
        shared().mutex.lock();

        if ((!shared().cursorChangedBuffer && !shared().cursorChangedHotspot) || !pointer || !shared().cursorSurface)
        {
            shared().mutex.unlock();
            return;
        }

        if (shared().cursorChangedBuffer)
        {
            wl_surface_damage(shared().cursorSurface, 0, 0, 512, 512);
            wl_surface_set_buffer_scale(shared().cursorSurface, shared().bufferScale);
            wl_surface_commit(shared().cursorSurface);
        }

        wl_pointer_set_cursor(pointer,
                              enterSerial,
                              shared().cursorVisible ? shared().cursorSurface : NULL,
                              shared().cursorHotspot.x() / shared().bufferScale,
                              shared().cursorHotspot.y() / shared().bufferScale);

        shared().cursorChangedBuffer = false;
        shared().cursorChangedHotspot = false;

        shared().mutex.unlock();
    }

    static bool initWayland()
    {
        setupListeners();
        display = static_cast<wl_display*>(Louvre::seat()->graphicBackendContextHandle());
        queue = wl_display_create_queue(display);
        registry = wl_display_get_registry(display);
        wl_proxy_set_queue((wl_proxy*)registry, queue);
        wl_registry_add_listener(registry, &registryListener, nullptr);

        if (shared().cursorSurface)
        {
            wl_proxy_set_queue((wl_proxy*)shared().cursorBuffer, queue);
            wl_proxy_set_queue((wl_proxy*)shared().cursorSurface, queue);
        }

        wl_display_roundtrip_queue(display, queue);

        if (!seat)
        {
            LLog::error("[%s] Failed to get wl_seat v1.", BKND_NAME);
            return false;
        }

        shared().fd[2].fd = eventfd(0, O_CLOEXEC | O_NONBLOCK);

        eventSource = compositor()->addFdListener(
            shared().fd[2].fd,
            nullptr,
            LInputBackend::processInput,
            POLLIN);

        // TODO
        compositor()->addFdListener(
            shared().fd[1].fd,
            nullptr,
            LInputBackend::processInput,
            POLLIN);

        return true;
    }

    static bool backendInitialize()
    {
        if (Louvre::seat()->graphicBackendId() != LGraphicBackendWayland)
        {
            LLog::error("[%s] The Wayland input backend requires the Wayland graphic backend.", BKND_NAME);
            return false;
        }

        Louvre::seat()->imp()->inputBackendData = &shared();

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
        if (eventSource)
            compositor()->removeFdListener(eventSource);
    }

    static void backendSuspend()
    {
        if (eventSource)
        {
            compositor()->removeFdListener(eventSource);
            eventSource = nullptr;
        }
    }

    static void backendResume()
    {
        if (!eventSource)
        {
            eventSource = compositor()->addFdListener(
                wl_display_get_fd(display),
                nullptr,
                LInputBackend::processInput,
                POLLIN);
        }
    }

    static void backendForceUpdate()
    {
        processInput(-1, 0, nullptr);
    }

    static Int32 processInput(Int32 fd, UInt32 /*mask*/, void *)
    {
        if (fd == shared().fd[2].fd)
        {
            eventfd_t val;
            eventfd_read(fd, &val);
        }
        else
        {
            while (wl_display_prepare_read_queue(display, queue) != 0)
                wl_display_dispatch_queue_pending(display, queue);
            wl_display_flush(display);

            unlockRenderThread();

            pollfd f = shared().fd[1];

            poll(&f, 1, 1);

            if (f.revents & POLLIN)
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

    static void registryHandleGlobalRemove(void */*data*/, struct wl_registry */*registry*/, uint32_t /*name*/)
    {
        // TODO
    }

    static void seatHandleCapabilities(void *, wl_seat *, UInt32 capabilities)
    {
        if (!pointer && capabilities & LSeat::InputCapabilities::Pointer)
        {
            pointer = wl_seat_get_pointer(seat);
            wl_proxy_set_queue((wl_proxy*)pointer, queue);
            wl_pointer_add_listener(pointer, &pointerListener, nullptr);
        }
    }

    static void seatHandleName(void *, wl_seat *, const char *) {}

    static void pointerHandleEnter(void *, wl_pointer *, UInt32 serial, wl_surface *, Float24 /*x*/, Float24 /*y*/)
    {
        enterSerial = serial;
        shared().cursorChangedBuffer = true;
    }

    static void pointerHandleLeave(void *, wl_pointer *, UInt32 /*serial*/, wl_surface *) {}

    static void pointerHandleMotion(void *, wl_pointer *, UInt32 time, Float24 x, Float24 y)
    {
        LPointF delta { (Float32)wl_fixed_to_double(x), (Float32)wl_fixed_to_double(y) };
        //delta *= (Float32)shared().bufferScale;
        pointerMoveEvent.setDelta(delta - cursor()->pos());
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
        Float32 val = wl_fixed_to_double(value);
        pointerScrollEvent.setSerial(LTime::nextSerial());
        pointerScrollEvent.setMs(time);
        pointerScrollEvent.setUs(LTime::us());

        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL)
            pointerScrollEvent.setX(val);
        else
            pointerScrollEvent.setY(val);

        pointerScrollEvent.notify();
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
