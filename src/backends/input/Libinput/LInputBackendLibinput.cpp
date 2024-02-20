#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LInputBackend.h>
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
#include <LLog.h>

#include <unordered_map>
#include <cstring>
#include <libinput.h>
#include <fcntl.h>

using namespace Louvre;

struct DEVICE_FD_ID
{
    Int32 fd;
    Int32 id;
};

struct BACKEND_DATA
{
    libinput *li { nullptr };
    udev *ud { nullptr };
    libinput_interface libinputInterface;
    LSeat *seat;
    std::list<DEVICE_FD_ID> devices;
    std::vector<LInputDevice*> pluggedDevices;
    std::vector<LInputDevice*> unpluggedDevices;
    LSeat::InputCapabilitiesFlags capabilities { 0 };

    // Recycled events
    LPointerMoveEvent pointerMoveEvent;
    LPointerButtonEvent pointerButtonEvent;
    LPointerScrollEvent pointerScrollEvent;
    LPointerSwipeBeginEvent pointerSwipeBeginEvent;
    LPointerSwipeUpdateEvent pointerSwipeUpdateEvent;
    LPointerSwipeEndEvent pointerSwipeEndEvent;
    LPointerPinchBeginEvent pointerPinchBeginEvent;
    LPointerPinchUpdateEvent pointerPinchUpdateEvent;
    LPointerPinchEndEvent pointerPinchEndEvent;
    LPointerHoldBeginEvent pointerHoldBeginEvent;
    LPointerHoldEndEvent pointerHoldEndEvent;
    LKeyboardKeyEvent keyboardKeyEvent;
    LTouchDownEvent touchDownEvent;
    LTouchMoveEvent touchMoveEvent;
    LTouchUpEvent touchUpEvent;
    LTouchFrameEvent touchFrameEvent;
    LTouchCancelEvent touchCancelEvent;
};

// Libseat devices
static bool libseatEnabled { false };
static wl_event_source *eventSource { nullptr };

// Event common
static libinput_device *dev;
static libinput_event *ev;
static libinput_event_type eventType;
static libinput_event_keyboard *keyEvent;
static libinput_event_pointer *pointerEvent;
static libinput_event_gesture *gestureEvent;
static libinput_event_touch *touchEvent;
static LInputDevice *inputDevice;

static Int32 openRestricted(const char *path, int flags, void *data)
{
    BACKEND_DATA *bknd { (BACKEND_DATA*)data };

    if (libseatEnabled)
    {
        DEVICE_FD_ID dev;
        dev.id = bknd->seat->openDevice(path, &dev.fd);

        if (dev.id == -1)
            return -1;

        bknd->devices.push_back(dev);
        return dev.fd;
    }
    else
    {   Int32 fd { open(path, flags) };
        return fd;
    }
}

static void closeRestricted(int fd, void *data)
{
    BACKEND_DATA *bknd { (BACKEND_DATA*)data };

    if (libseatEnabled)
    {
        DEVICE_FD_ID dev {-1, -1};

        for (std::list<DEVICE_FD_ID>::iterator it = bknd->devices.begin(); it != bknd->devices.end(); it++)
        {
            if ((*it).fd == fd)
            {
                dev = (*it);
                bknd->devices.erase(it);
                break;
            }
        }

        if (dev.fd == -1)
            return;

        bknd->seat->closeDevice(dev.id);
    }

    close(fd);
}

/****************** UTILITIES ******************/

static void updateCapabilities(BACKEND_DATA *bknd)
{
    bknd->capabilities = 0;

    for (const LInputDevice *device : bknd->pluggedDevices)
        bknd->capabilities |= device->capabilities();
}

/****************** DEVICE INTERFACE ******************/

static LSeat::InputCapabilitiesFlags deviceCapabilities(libinput_device *dev)
{
    LSeat::InputCapabilitiesFlags caps { 0 };

    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_POINTER))
        caps |= LSeat::Pointer;

    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_KEYBOARD))
        caps |= LSeat::Keyboard;

    if (libinput_device_has_capability(dev, LIBINPUT_DEVICE_CAP_TOUCH))
        caps |= LSeat::Touch;

    return caps;
}

Int32 LInputBackend::processInput(int, unsigned int, void *userData)
{
    LSeat *seat { (LSeat*)userData };
    BACKEND_DATA *data { (BACKEND_DATA*)seat->imp()->inputBackendData };

    const Int32 ret { libinput_dispatch(data->li) };

    if (ret != 0)
    {
        LLog::error("[Libinput Backend] Failed to dispatch libinput %s.", strerror(-ret));
        return 0;
    }

    while ((ev = libinput_get_event(data->li)) != NULL)
    {
        eventType = libinput_event_get_type(ev);

        switch (eventType)
        {
        case LIBINPUT_EVENT_POINTER_MOTION:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            pointerEvent = libinput_event_get_pointer_event(ev);
            data->pointerMoveEvent.setDevice(inputDevice);
            data->pointerMoveEvent.setDx(libinput_event_pointer_get_dx(pointerEvent));
            data->pointerMoveEvent.setDy(libinput_event_pointer_get_dy(pointerEvent));
            data->pointerMoveEvent.setDxUnaccelerated(libinput_event_pointer_get_dx_unaccelerated(pointerEvent));
            data->pointerMoveEvent.setDyUnaccelerated(libinput_event_pointer_get_dy_unaccelerated(pointerEvent));
            data->pointerMoveEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
            data->pointerMoveEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
            data->pointerMoveEvent.setSerial(LTime::nextSerial());
            data->pointerMoveEvent.notify();
            break;
        case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            pointerEvent = libinput_event_get_pointer_event(ev);
            data->pointerScrollEvent.setDevice(inputDevice);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
                data->pointerScrollEvent.setX(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
                data->pointerScrollEvent.setY(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));

            data->pointerScrollEvent.set120X(0.f);
            data->pointerScrollEvent.set120Y(0.f);
            data->pointerScrollEvent.setSource(LPointerScrollEvent::Finger);
            data->pointerScrollEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
            data->pointerScrollEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
            data->pointerScrollEvent.setSerial(LTime::nextSerial());
            data->pointerScrollEvent.notify();
            break;
        case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            pointerEvent = libinput_event_get_pointer_event(ev);
            data->pointerScrollEvent.setDevice(inputDevice);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
                data->pointerScrollEvent.setX(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
                data->pointerScrollEvent.setY(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));

            data->pointerScrollEvent.set120X(0.f);
            data->pointerScrollEvent.set120Y(0.f);
            data->pointerScrollEvent.setSource(LPointerScrollEvent::Continuous);
            data->pointerScrollEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
            data->pointerScrollEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
            data->pointerScrollEvent.setSerial(LTime::nextSerial());
            data->pointerScrollEvent.notify();
            break;
        case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            pointerEvent = libinput_event_get_pointer_event(ev);
            data->pointerScrollEvent.setDevice(inputDevice);

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
            {
                data->pointerScrollEvent.setX(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                data->pointerScrollEvent.set120X(libinput_event_pointer_get_scroll_value_v120(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
            }

            if (libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
            {
                data->pointerScrollEvent.setY(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                data->pointerScrollEvent.set120Y(libinput_event_pointer_get_scroll_value_v120(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
            }

            data->pointerScrollEvent.setSource(LPointerScrollEvent::Wheel);
            data->pointerScrollEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
            data->pointerScrollEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
            data->pointerScrollEvent.setSerial(LTime::nextSerial());
            data->pointerScrollEvent.notify();
            break;
        case LIBINPUT_EVENT_POINTER_BUTTON:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            pointerEvent = libinput_event_get_pointer_event(ev);
            data->pointerButtonEvent.setDevice(inputDevice);
            data->pointerButtonEvent.setButton((LPointerButtonEvent::Button)libinput_event_pointer_get_button(pointerEvent));
            data->pointerButtonEvent.setState((LPointerButtonEvent::State)libinput_event_pointer_get_button_state(pointerEvent));
            data->pointerButtonEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
            data->pointerButtonEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
            data->pointerButtonEvent.setSerial(LTime::nextSerial());
            data->pointerButtonEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerSwipeBeginEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerSwipeBeginEvent.setDevice(inputDevice);
            data->pointerSwipeBeginEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerSwipeBeginEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerSwipeBeginEvent.setSerial(LTime::nextSerial());
            data->pointerSwipeBeginEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerSwipeUpdateEvent.setDx(libinput_event_gesture_get_dx(gestureEvent));
            data->pointerSwipeUpdateEvent.setDy(libinput_event_gesture_get_dy(gestureEvent));
            data->pointerSwipeUpdateEvent.setDxUnaccelerated(libinput_event_gesture_get_dx_unaccelerated(gestureEvent));
            data->pointerSwipeUpdateEvent.setDyUnaccelerated(libinput_event_gesture_get_dy_unaccelerated(gestureEvent));
            data->pointerSwipeUpdateEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerSwipeUpdateEvent.setDevice(inputDevice);
            data->pointerSwipeUpdateEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerSwipeUpdateEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerSwipeUpdateEvent.setSerial(LTime::nextSerial());
            data->pointerSwipeUpdateEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_SWIPE_END:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerSwipeEndEvent.setCancelled(libinput_event_gesture_get_cancelled(gestureEvent));
            data->pointerSwipeEndEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerSwipeEndEvent.setDevice(inputDevice);
            data->pointerSwipeEndEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerSwipeEndEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerSwipeEndEvent.setSerial(LTime::nextSerial());
            data->pointerSwipeEndEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerPinchBeginEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerPinchBeginEvent.setDevice(inputDevice);
            data->pointerPinchBeginEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerPinchBeginEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerPinchBeginEvent.setSerial(LTime::nextSerial());
            data->pointerPinchBeginEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerPinchUpdateEvent.setDx(libinput_event_gesture_get_dx(gestureEvent));
            data->pointerPinchUpdateEvent.setDy(libinput_event_gesture_get_dy(gestureEvent));
            data->pointerPinchUpdateEvent.setDxUnaccelerated(libinput_event_gesture_get_dx_unaccelerated(gestureEvent));
            data->pointerPinchUpdateEvent.setDyUnaccelerated(libinput_event_gesture_get_dy_unaccelerated(gestureEvent));
            data->pointerPinchUpdateEvent.setScale(libinput_event_gesture_get_scale(gestureEvent));
            data->pointerPinchUpdateEvent.setRotation(libinput_event_gesture_get_angle_delta(gestureEvent));
            data->pointerPinchUpdateEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerPinchUpdateEvent.setDevice(inputDevice);
            data->pointerPinchUpdateEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerPinchUpdateEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerPinchUpdateEvent.setSerial(LTime::nextSerial());
            data->pointerPinchUpdateEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_PINCH_END:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerPinchEndEvent.setCancelled(libinput_event_gesture_get_cancelled(gestureEvent));
            data->pointerPinchEndEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerPinchEndEvent.setDevice(inputDevice);
            data->pointerPinchEndEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerPinchEndEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerPinchEndEvent.setSerial(LTime::nextSerial());
            data->pointerPinchEndEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerHoldBeginEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerHoldBeginEvent.setDevice(inputDevice);
            data->pointerHoldBeginEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerHoldBeginEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerHoldBeginEvent.setSerial(LTime::nextSerial());
            data->pointerHoldBeginEvent.notify();
            break;
        case LIBINPUT_EVENT_GESTURE_HOLD_END:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            gestureEvent = libinput_event_get_gesture_event(ev);
            data->pointerHoldEndEvent.setCancelled(libinput_event_gesture_get_cancelled(gestureEvent));
            data->pointerHoldEndEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
            data->pointerHoldEndEvent.setDevice(inputDevice);
            data->pointerHoldEndEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
            data->pointerHoldEndEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
            data->pointerHoldEndEvent.setSerial(LTime::nextSerial());
            data->pointerHoldEndEvent.notify();
            break;
        case LIBINPUT_EVENT_KEYBOARD_KEY:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            keyEvent = libinput_event_get_keyboard_event(ev);
            data->keyboardKeyEvent.setDevice(inputDevice);
            data->keyboardKeyEvent.setKeyCode(libinput_event_keyboard_get_key(keyEvent));
            data->keyboardKeyEvent.setState((LKeyboardKeyEvent::State)libinput_event_keyboard_get_key_state(keyEvent));
            data->keyboardKeyEvent.setMs(libinput_event_keyboard_get_time(keyEvent));
            data->keyboardKeyEvent.setUs(libinput_event_keyboard_get_time_usec(keyEvent));
            data->keyboardKeyEvent.setSerial(LTime::nextSerial());
            data->keyboardKeyEvent.notify();
            break;
        case LIBINPUT_EVENT_TOUCH_DOWN:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            touchEvent = libinput_event_get_touch_event(ev);
            data->touchDownEvent.setDevice(inputDevice);
            data->touchDownEvent.setX(libinput_event_touch_get_x_transformed(touchEvent, 1));
            data->touchDownEvent.setY(libinput_event_touch_get_y_transformed(touchEvent, 1));
            data->touchDownEvent.setId(libinput_event_touch_get_seat_slot(touchEvent));
            data->touchDownEvent.setMs(libinput_event_touch_get_time(touchEvent));
            data->touchDownEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
            data->touchDownEvent.setSerial(LTime::nextSerial());
            data->touchDownEvent.notify();
            break;
        case LIBINPUT_EVENT_TOUCH_MOTION:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            touchEvent = libinput_event_get_touch_event(ev);
            data->touchMoveEvent.setDevice(inputDevice);
            data->touchMoveEvent.setX(libinput_event_touch_get_x_transformed(touchEvent, 1));
            data->touchMoveEvent.setY(libinput_event_touch_get_y_transformed(touchEvent, 1));
            data->touchMoveEvent.setId(libinput_event_touch_get_seat_slot(touchEvent));
            data->touchMoveEvent.setMs(libinput_event_touch_get_time(touchEvent));
            data->touchMoveEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
            data->touchMoveEvent.setSerial(LTime::nextSerial());
            data->touchMoveEvent.notify();
            break;
        case LIBINPUT_EVENT_TOUCH_UP:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            touchEvent = libinput_event_get_touch_event(ev);
            data->touchUpEvent.setDevice(inputDevice);
            data->touchUpEvent.setId(libinput_event_touch_get_seat_slot(touchEvent));
            data->touchUpEvent.setMs(libinput_event_touch_get_time(touchEvent));
            data->touchUpEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
            data->touchUpEvent.setSerial(LTime::nextSerial());
            data->touchUpEvent.notify();
            break;
        case LIBINPUT_EVENT_TOUCH_FRAME:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            touchEvent = libinput_event_get_touch_event(ev);
            data->touchFrameEvent.setDevice(inputDevice);
            data->touchFrameEvent.setMs(libinput_event_touch_get_time(touchEvent));
            data->touchFrameEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
            data->touchFrameEvent.setSerial(LTime::nextSerial());
            data->touchFrameEvent.notify();
            break;
        case LIBINPUT_EVENT_TOUCH_CANCEL:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            touchEvent = libinput_event_get_touch_event(ev);
            data->touchCancelEvent.setDevice(inputDevice);
            data->touchCancelEvent.setMs(libinput_event_touch_get_time(touchEvent));
            data->touchCancelEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
            data->touchCancelEvent.setSerial(LTime::nextSerial());
            data->touchCancelEvent.notify();
            break;
        case LIBINPUT_EVENT_DEVICE_ADDED:
            dev = libinput_event_get_device(ev);
            inputDevice = nullptr;

            for (LInputDevice *idev : data->unpluggedDevices)
            {
                if (idev->name() == libinput_device_get_name(dev) && idev->vendorId() == libinput_device_get_id_vendor(dev) && idev->productId() == libinput_device_get_id_product(dev))
                {
                    inputDevice = idev;
                    LVectorRemoveOneUnordered(data->unpluggedDevices, idev);
                    break;
                }
            }

            if (!inputDevice)
                inputDevice = new LInputDevice();

            libinput_device_set_user_data(dev, inputDevice);
            inputDevice->m_backendData = dev;
            inputDevice->m_capabilities = deviceCapabilities(dev);
            inputDevice->m_name = libinput_device_get_name(dev);
            inputDevice->m_vendorId = libinput_device_get_id_vendor(dev);
            inputDevice->m_productId = libinput_device_get_id_product(dev);
            data->pluggedDevices.push_back(inputDevice);
            data->capabilities |= inputDevice->capabilities();
            inputDevice->notifyPlugged();
            break;
        case LIBINPUT_EVENT_DEVICE_REMOVED:
            dev = libinput_event_get_device(ev);
            inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
            LVectorRemoveOneUnordered(data->pluggedDevices, inputDevice);
            data->unpluggedDevices.push_back(inputDevice);
            updateCapabilities(data);
            inputDevice->notifyUnplugged();
            break;
        default:
            break;
        }

        seat->nativeInputEvent(ev);
        libinput_event_destroy(ev);
    }

    return 0;
}

UInt32 LInputBackend::backendGetId()
{
    return LInputBackendLibinput;
}

bool LInputBackend::backendInitialize()
{
    Int32 fd;
    const LSeat *seat { LCompositor::compositor()->seat() };
    libseatEnabled = seat->imp()->initLibseat();

    BACKEND_DATA *data { new BACKEND_DATA };
    data->seat = (LSeat*)seat;
    seat->imp()->inputBackendData = data;
    data->ud = udev_new();

    if (!data->ud)
        goto fail;

    data->libinputInterface.open_restricted = &openRestricted;
    data->libinputInterface.close_restricted = &closeRestricted;
    data->li = libinput_udev_create_context(&data->libinputInterface, data, data->ud);

    if (!data->li)
        goto fail;

    if (libseatEnabled )
        libinput_udev_assign_seat(data->li, libseat_seat_name(seat->libseatHandle()));
    else
        libinput_udev_assign_seat(data->li, "seat0");

    fd = libinput_get_fd(data->li);
    eventSource = LCompositor::addFdListener(fd, (LSeat*)seat, &processInput);
    return true;

fail:
    backendUninitialize();
    return false;
}

UInt32 LInputBackend::backendGetCapabilities()
{
    const LSeat *seat { LCompositor::compositor()->seat() };
    const BACKEND_DATA *data { (BACKEND_DATA*)seat->imp()->inputBackendData };
    return data->capabilities;
}

void *LInputBackend::backendGetContextHandle()
{
    const LSeat *seat { LCompositor::compositor()->seat() };
    const BACKEND_DATA *data { (BACKEND_DATA*)seat->imp()->inputBackendData };
    return data->li;
}

void LInputBackend::backendSuspend()
{
    const LSeat *seat { LCompositor::compositor()->seat() };
    const BACKEND_DATA *data { (BACKEND_DATA*)seat->imp()->inputBackendData };
    libinput_suspend(data->li);
}

void LInputBackend::backendForceUpdate()
{
    processInput(0, 0, LCompositor::compositor()->seat());
}

void LInputBackend::backendSetLeds(UInt32 leds)
{
    const LSeat *seat { LCompositor::compositor()->seat() };
    const BACKEND_DATA *data { (BACKEND_DATA*)seat->imp()->inputBackendData };
    for (auto device : data->pluggedDevices)
        libinput_device_led_update((libinput_device*)device->m_backendData, (libinput_led)leds);
}

void LInputBackend::backendResume()
{
    const LSeat *seat { LCompositor::compositor()->seat() };
    const BACKEND_DATA *data { (BACKEND_DATA*)seat->imp()->inputBackendData };

    if (libinput_resume(data->li) == -1)
    {
        LLog::error("[Libinput Backend] Failed to resume libinput.");
        return;
    }
}

void LInputBackend::backendUninitialize()
{
    LSeat *seat = LCompositor::compositor()->seat();
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;

    if (!data)
        return;

    if (eventSource)
    {
        LCompositor::removeFdListener(eventSource);
        eventSource = nullptr;
    }

    // Only delete devices, do not notify
    while (!data->pluggedDevices.empty())
    {
        delete data->pluggedDevices.back();
        data->pluggedDevices.pop_back();
    }

    while (!data->unpluggedDevices.empty())
    {
        delete data->unpluggedDevices.back();
        data->unpluggedDevices.pop_back();
    }

    if (data->li)
        libinput_unref(data->li);

    if (data->ud)
        udev_unref(data->ud);

    delete data;
    seat->imp()->inputBackendData = nullptr;
}

const std::vector<LInputDevice *> *LInputBackend::backendGetDevices()
{
    const LSeat *seat { LCompositor::compositor()->seat() };
    const BACKEND_DATA *data { (BACKEND_DATA*)seat->imp()->inputBackendData };
    return &data->pluggedDevices;
}

LInputBackendInterface API;

extern "C" LInputBackendInterface *getAPI()
{
    API.backendGetId            = &LInputBackend::backendGetId;
    API.backendGetCapabilities  = &LInputBackend::backendGetCapabilities;
    API.backendGetContextHandle = &LInputBackend::backendGetContextHandle;
    API.backendGetDevices       = &LInputBackend::backendGetDevices;
    API.backendInitialize       = &LInputBackend::backendInitialize;
    API.backendUninitialize     = &LInputBackend::backendUninitialize;
    API.backendSuspend          = &LInputBackend::backendSuspend;
    API.backendResume           = &LInputBackend::backendResume;
    API.backendSetLeds          = &LInputBackend::backendSetLeds;
    API.backendForceUpdate      = &LInputBackend::backendForceUpdate;
    return &API;
}
