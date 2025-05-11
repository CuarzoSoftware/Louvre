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
#include <LCursor.h>
#include <LUtils.h>
#include <LLog.h>

#include <cstring>
#include <libinput.h>
#include <fcntl.h>

using namespace Louvre;

struct DEVICE_FD_ID
{
    Int32 fd;
    Int32 id;
};

class Louvre::LInputBackend
{
public:
    static inline libinput *li { nullptr };
    static inline udev *ud { nullptr };
    static inline libinput_interface libinputInterface;
    static inline std::list<DEVICE_FD_ID> devices;
    static inline std::vector<LInputDevice*> pluggedDevices;
    static inline std::vector<LInputDevice*> unpluggedDevices;

    // Libseat devices
    static inline bool libseatEnabled { false };
    static inline wl_event_source *eventSource { nullptr };

    // Event common
    static inline libinput_device *dev;
    static inline libinput_event *ev;
    static inline libinput_event_type eventType;
    static inline libinput_event_keyboard *keyEvent;
    static inline libinput_event_pointer *pointerEvent;
    static inline libinput_event_gesture *gestureEvent;
    static inline libinput_event_touch *touchEvent;
    static inline LInputDevice *inputDevice;

    // Recycled events
    static inline LPointerMoveEvent pointerMoveEvent;
    static inline LPointerButtonEvent pointerButtonEvent;
    static inline LPointerScrollEvent pointerScrollEvent;
    static inline LPointerSwipeBeginEvent pointerSwipeBeginEvent;
    static inline LPointerSwipeUpdateEvent pointerSwipeUpdateEvent;
    static inline LPointerSwipeEndEvent pointerSwipeEndEvent;
    static inline LPointerPinchBeginEvent pointerPinchBeginEvent;
    static inline LPointerPinchUpdateEvent pointerPinchUpdateEvent;
    static inline LPointerPinchEndEvent pointerPinchEndEvent;
    static inline LPointerHoldBeginEvent pointerHoldBeginEvent;
    static inline LPointerHoldEndEvent pointerHoldEndEvent;
    static inline LKeyboardKeyEvent keyboardKeyEvent;
    static inline LTouchDownEvent touchDownEvent;
    static inline LTouchMoveEvent touchMoveEvent;
    static inline LTouchUpEvent touchUpEvent;
    static inline LTouchFrameEvent touchFrameEvent;
    static inline LTouchCancelEvent touchCancelEvent;
    static inline Float32 dx, dy;

    static Int32 openRestricted(const char *path, int flags, void */*data*/)
    {
        if (libseatEnabled)
        {
            DEVICE_FD_ID dev;
            dev.id = seat()->openDevice(path, &dev.fd);

            if (dev.id == -1)
                return -1;

            devices.push_back(dev);
            return dev.fd;
        }
        else
        {   Int32 fd { open(path, flags) };
            return fd;
        }
    }

    static void closeRestricted(int fd, void */*data*/)
    {
        if (libseatEnabled)
        {
            DEVICE_FD_ID dev {-1, -1};

            for (std::list<DEVICE_FD_ID>::iterator it = devices.begin(); it != devices.end(); it++)
            {
                if ((*it).fd == fd)
                {
                    dev = (*it);
                    devices.erase(it);
                    break;
                }
            }

            if (dev.fd == -1)
                return;

            seat()->closeDevice(dev.id);
        }

        close(fd);
    }

    static UInt32 deviceCapabilities(libinput_device *dev)
    {
        UInt32 caps { 0 };

        for (UInt32 cap = 0; cap <= 6; cap++)
        {
            if (libinput_device_has_capability(dev, (libinput_device_capability)cap))
                caps |= (1 << cap);
        }

        return caps;
    }

    static Int32 processInput(int, unsigned int, void *)
    {
        const Int32 ret { libinput_dispatch(li) };

        if (ret != 0)
        {
            LLog::error("[Libinput Backend] Failed to dispatch libinput %s.", strerror(-ret));
            return 0;
        }

        while ((ev = libinput_get_event(li)) != NULL)
        {
            eventType = libinput_event_get_type(ev);

            switch (eventType)
            {
            case LIBINPUT_EVENT_POINTER_MOTION:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                pointerEvent = libinput_event_get_pointer_event(ev);
                pointerMoveEvent.setDevice(inputDevice);
                pointerMoveEvent.setDx(libinput_event_pointer_get_dx(pointerEvent));
                pointerMoveEvent.setDy(libinput_event_pointer_get_dy(pointerEvent));
                pointerMoveEvent.setDxUnaccelerated(libinput_event_pointer_get_dx_unaccelerated(pointerEvent));
                pointerMoveEvent.setDyUnaccelerated(libinput_event_pointer_get_dy_unaccelerated(pointerEvent));
                pointerMoveEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
                pointerMoveEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
                pointerMoveEvent.setSerial(LTime::nextSerial());
                pointerMoveEvent.notify();
                break;
            case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                pointerEvent = libinput_event_get_pointer_event(ev);
                pointerMoveEvent.setDevice(inputDevice);

                if (cursor() && cursor()->output())
                {
                    dx = Float32(cursor()->output()->pos().x()) +
                                 libinput_event_pointer_get_absolute_x_transformed(pointerEvent, cursor()->output()->size().w()) -
                                 cursor()->pos().x();
                    dy = Float32(cursor()->output()->pos().y()) +
                                 libinput_event_pointer_get_absolute_y_transformed(pointerEvent, cursor()->output()->size().h()) -
                                 cursor()->pos().y();
                }
                else
                    dx = dy = 0.f;

                pointerMoveEvent.setDx(dx);
                pointerMoveEvent.setDy(dy);
                pointerMoveEvent.setDxUnaccelerated(dx);
                pointerMoveEvent.setDyUnaccelerated(dy);
                pointerMoveEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
                pointerMoveEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
                pointerMoveEvent.setSerial(LTime::nextSerial());
                pointerMoveEvent.notify();
                break;
            // Legacy wheel scroll events
            case LIBINPUT_EVENT_POINTER_AXIS:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                pointerEvent = libinput_event_get_pointer_event(ev);
                pointerScrollEvent.setDevice(inputDevice);
                pointerScrollEvent.setHasX(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0);
                pointerScrollEvent.setHasY(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0);
                pointerScrollEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
                pointerScrollEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
                pointerScrollEvent.setSerial(LTime::nextSerial());

                if (libinput_event_pointer_get_axis_source(pointerEvent) == LIBINPUT_POINTER_AXIS_SOURCE_WHEEL)
                {
                    if (pointerScrollEvent.hasX())
                    {
                        pointerScrollEvent.setX(libinput_event_pointer_get_axis_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                        pointerScrollEvent.setDiscreteX(libinput_event_pointer_get_axis_value_discrete(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                    }
                    else
                    {
                        pointerScrollEvent.setX(0.f);
                        pointerScrollEvent.setDiscreteX(0);
                    }

                    if (pointerScrollEvent.hasY())
                    {
                        pointerScrollEvent.setY(libinput_event_pointer_get_axis_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                        pointerScrollEvent.setDiscreteY(libinput_event_pointer_get_axis_value_discrete(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                    }
                    else
                    {
                        pointerScrollEvent.setY(0.f);
                        pointerScrollEvent.setDiscreteY(0);
                    }

                    pointerScrollEvent.setSource(LPointerScrollEvent::WheelLegacy);
                    pointerScrollEvent.notify();
                }
                else if (libinput_event_pointer_get_axis_source(pointerEvent) == LIBINPUT_POINTER_AXIS_SOURCE_WHEEL_TILT)
                {
                    if (pointerScrollEvent.hasX())
                        pointerScrollEvent.setX(libinput_event_pointer_get_axis_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                    else
                        pointerScrollEvent.setX(0.f);

                    if (pointerScrollEvent.hasY())
                        pointerScrollEvent.setY(libinput_event_pointer_get_axis_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                    else
                        pointerScrollEvent.setY(0.f);

                    pointerScrollEvent.setDiscreteAxes(0, 0);
                    pointerScrollEvent.setSource(LPointerScrollEvent::WheelTilt);
                    pointerScrollEvent.notify();
                }
                break;
            case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                pointerEvent = libinput_event_get_pointer_event(ev);
                pointerScrollEvent.setDevice(inputDevice);
                pointerScrollEvent.setHasX(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0);
                pointerScrollEvent.setHasY(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0);

                if (pointerScrollEvent.hasX())
                    pointerScrollEvent.setX(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                else
                    pointerScrollEvent.setX(0.f);

                if (pointerScrollEvent.hasY())
                    pointerScrollEvent.setY(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                else
                    pointerScrollEvent.setY(0.f);

                pointerScrollEvent.setDiscreteAxes(0, 0);
                pointerScrollEvent.setSource(LPointerScrollEvent::Finger);
                pointerScrollEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
                pointerScrollEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
                pointerScrollEvent.setSerial(LTime::nextSerial());
                pointerScrollEvent.notify();
                break;
            case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                pointerEvent = libinput_event_get_pointer_event(ev);
                pointerScrollEvent.setDevice(inputDevice);

                pointerScrollEvent.setHasX(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0);
                pointerScrollEvent.setHasY(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0);

                if (pointerScrollEvent.hasX())
                    pointerScrollEvent.setX(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                else
                    pointerScrollEvent.setX(0.f);

                if (pointerScrollEvent.hasY())
                    pointerScrollEvent.setY(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                else
                    pointerScrollEvent.setY(0.f);

                pointerScrollEvent.setDiscreteAxes(0, 0);
                pointerScrollEvent.setSource(LPointerScrollEvent::Continuous);
                pointerScrollEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
                pointerScrollEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
                pointerScrollEvent.setSerial(LTime::nextSerial());
                pointerScrollEvent.notify();
                break;
            case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                pointerEvent = libinput_event_get_pointer_event(ev);
                pointerScrollEvent.setDevice(inputDevice);

                pointerScrollEvent.setHasX(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0);
                pointerScrollEvent.setHasY(libinput_event_pointer_has_axis(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0);

                if (pointerScrollEvent.hasX())
                {
                    pointerScrollEvent.setX(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                    pointerScrollEvent.setDiscreteX(libinput_event_pointer_get_scroll_value_v120(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL));
                }
                else
                {
                    pointerScrollEvent.setX(0.f);
                    pointerScrollEvent.setDiscreteX(0);
                }

                if (pointerScrollEvent.hasY())
                {
                    pointerScrollEvent.setY(libinput_event_pointer_get_scroll_value(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                    pointerScrollEvent.setDiscreteY(libinput_event_pointer_get_scroll_value_v120(pointerEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL));
                }
                else
                {
                    pointerScrollEvent.setY(0.f);
                    pointerScrollEvent.setDiscreteY(0);
                }

                pointerScrollEvent.setSource(LPointerScrollEvent::Wheel);
                pointerScrollEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
                pointerScrollEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
                pointerScrollEvent.setSerial(LTime::nextSerial());
                pointerScrollEvent.notify();
                break;
            case LIBINPUT_EVENT_POINTER_BUTTON:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                pointerEvent = libinput_event_get_pointer_event(ev);
                pointerButtonEvent.setDevice(inputDevice);
                pointerButtonEvent.setButton((LPointerButtonEvent::Button)libinput_event_pointer_get_button(pointerEvent));
                pointerButtonEvent.setState((LPointerButtonEvent::State)libinput_event_pointer_get_button_state(pointerEvent));
                pointerButtonEvent.setMs(libinput_event_pointer_get_time(pointerEvent));
                pointerButtonEvent.setUs(libinput_event_pointer_get_time_usec(pointerEvent));
                pointerButtonEvent.setSerial(LTime::nextSerial());
                pointerButtonEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerSwipeBeginEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerSwipeBeginEvent.setDevice(inputDevice);
                pointerSwipeBeginEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerSwipeBeginEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerSwipeBeginEvent.setSerial(LTime::nextSerial());
                pointerSwipeBeginEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerSwipeUpdateEvent.setDx(libinput_event_gesture_get_dx(gestureEvent));
                pointerSwipeUpdateEvent.setDy(libinput_event_gesture_get_dy(gestureEvent));
                pointerSwipeUpdateEvent.setDxUnaccelerated(libinput_event_gesture_get_dx_unaccelerated(gestureEvent));
                pointerSwipeUpdateEvent.setDyUnaccelerated(libinput_event_gesture_get_dy_unaccelerated(gestureEvent));
                pointerSwipeUpdateEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerSwipeUpdateEvent.setDevice(inputDevice);
                pointerSwipeUpdateEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerSwipeUpdateEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerSwipeUpdateEvent.setSerial(LTime::nextSerial());
                pointerSwipeUpdateEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_SWIPE_END:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerSwipeEndEvent.setCancelled(libinput_event_gesture_get_cancelled(gestureEvent));
                pointerSwipeEndEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerSwipeEndEvent.setDevice(inputDevice);
                pointerSwipeEndEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerSwipeEndEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerSwipeEndEvent.setSerial(LTime::nextSerial());
                pointerSwipeEndEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerPinchBeginEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerPinchBeginEvent.setDevice(inputDevice);
                pointerPinchBeginEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerPinchBeginEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerPinchBeginEvent.setSerial(LTime::nextSerial());
                pointerPinchBeginEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerPinchUpdateEvent.setDx(libinput_event_gesture_get_dx(gestureEvent));
                pointerPinchUpdateEvent.setDy(libinput_event_gesture_get_dy(gestureEvent));
                pointerPinchUpdateEvent.setDxUnaccelerated(libinput_event_gesture_get_dx_unaccelerated(gestureEvent));
                pointerPinchUpdateEvent.setDyUnaccelerated(libinput_event_gesture_get_dy_unaccelerated(gestureEvent));
                pointerPinchUpdateEvent.setScale(libinput_event_gesture_get_scale(gestureEvent));
                pointerPinchUpdateEvent.setRotation(libinput_event_gesture_get_angle_delta(gestureEvent));
                pointerPinchUpdateEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerPinchUpdateEvent.setDevice(inputDevice);
                pointerPinchUpdateEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerPinchUpdateEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerPinchUpdateEvent.setSerial(LTime::nextSerial());
                pointerPinchUpdateEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_PINCH_END:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerPinchEndEvent.setCancelled(libinput_event_gesture_get_cancelled(gestureEvent));
                pointerPinchEndEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerPinchEndEvent.setDevice(inputDevice);
                pointerPinchEndEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerPinchEndEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerPinchEndEvent.setSerial(LTime::nextSerial());
                pointerPinchEndEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerHoldBeginEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerHoldBeginEvent.setDevice(inputDevice);
                pointerHoldBeginEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerHoldBeginEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerHoldBeginEvent.setSerial(LTime::nextSerial());
                pointerHoldBeginEvent.notify();
                break;
            case LIBINPUT_EVENT_GESTURE_HOLD_END:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                gestureEvent = libinput_event_get_gesture_event(ev);
                pointerHoldEndEvent.setCancelled(libinput_event_gesture_get_cancelled(gestureEvent));
                pointerHoldEndEvent.setFingers(libinput_event_gesture_get_finger_count(gestureEvent));
                pointerHoldEndEvent.setDevice(inputDevice);
                pointerHoldEndEvent.setMs(libinput_event_gesture_get_time(gestureEvent));
                pointerHoldEndEvent.setUs(libinput_event_gesture_get_time_usec(gestureEvent));
                pointerHoldEndEvent.setSerial(LTime::nextSerial());
                pointerHoldEndEvent.notify();
                break;
            case LIBINPUT_EVENT_KEYBOARD_KEY:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                keyEvent = libinput_event_get_keyboard_event(ev);
                keyboardKeyEvent.setDevice(inputDevice);
                keyboardKeyEvent.setKeyCode(libinput_event_keyboard_get_key(keyEvent));
                keyboardKeyEvent.setState((LKeyboardKeyEvent::State)libinput_event_keyboard_get_key_state(keyEvent));
                keyboardKeyEvent.setMs(libinput_event_keyboard_get_time(keyEvent));
                keyboardKeyEvent.setUs(libinput_event_keyboard_get_time_usec(keyEvent));
                keyboardKeyEvent.setSerial(LTime::nextSerial());
                keyboardKeyEvent.notify();
                break;
            case LIBINPUT_EVENT_TOUCH_DOWN:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                touchEvent = libinput_event_get_touch_event(ev);
                touchDownEvent.setDevice(inputDevice);
                touchDownEvent.setX(libinput_event_touch_get_x_transformed(touchEvent, 1));
                touchDownEvent.setY(libinput_event_touch_get_y_transformed(touchEvent, 1));
                touchDownEvent.setId(libinput_event_touch_get_seat_slot(touchEvent));
                touchDownEvent.setMs(libinput_event_touch_get_time(touchEvent));
                touchDownEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
                touchDownEvent.setSerial(LTime::nextSerial());
                touchDownEvent.notify();
                break;
            case LIBINPUT_EVENT_TOUCH_MOTION:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                touchEvent = libinput_event_get_touch_event(ev);
                touchMoveEvent.setDevice(inputDevice);
                touchMoveEvent.setX(libinput_event_touch_get_x_transformed(touchEvent, 1));
                touchMoveEvent.setY(libinput_event_touch_get_y_transformed(touchEvent, 1));
                touchMoveEvent.setId(libinput_event_touch_get_seat_slot(touchEvent));
                touchMoveEvent.setMs(libinput_event_touch_get_time(touchEvent));
                touchMoveEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
                touchMoveEvent.setSerial(LTime::nextSerial());
                touchMoveEvent.notify();
                break;
            case LIBINPUT_EVENT_TOUCH_UP:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                touchEvent = libinput_event_get_touch_event(ev);
                touchUpEvent.setDevice(inputDevice);
                touchUpEvent.setId(libinput_event_touch_get_seat_slot(touchEvent));
                touchUpEvent.setMs(libinput_event_touch_get_time(touchEvent));
                touchUpEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
                touchUpEvent.setSerial(LTime::nextSerial());
                touchUpEvent.notify();
                break;
            case LIBINPUT_EVENT_TOUCH_FRAME:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                touchEvent = libinput_event_get_touch_event(ev);
                touchFrameEvent.setDevice(inputDevice);
                touchFrameEvent.setMs(libinput_event_touch_get_time(touchEvent));
                touchFrameEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
                touchFrameEvent.setSerial(LTime::nextSerial());
                touchFrameEvent.notify();
                break;
            case LIBINPUT_EVENT_TOUCH_CANCEL:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                touchEvent = libinput_event_get_touch_event(ev);
                touchCancelEvent.setDevice(inputDevice);
                touchCancelEvent.setMs(libinput_event_touch_get_time(touchEvent));
                touchCancelEvent.setUs(libinput_event_touch_get_time_usec(touchEvent));
                touchCancelEvent.setSerial(LTime::nextSerial());
                touchCancelEvent.notify();
                break;
            case LIBINPUT_EVENT_DEVICE_ADDED:
                dev = libinput_event_get_device(ev);
                inputDevice = nullptr;

                for (LInputDevice *idev : unpluggedDevices)
                {
                    if (idev->name() == libinput_device_get_name(dev) && idev->vendorId() == libinput_device_get_id_vendor(dev) && idev->productId() == libinput_device_get_id_product(dev))
                    {
                        inputDevice = idev;
                        LVectorRemoveOneUnordered(unpluggedDevices, idev);
                        break;
                    }
                }

                if (!inputDevice)
                    inputDevice = new LInputDevice();

                libinput_device_set_user_data(dev, inputDevice);
                inputDevice->m_nativeHandle = dev;
                inputDevice->m_capabilities = deviceCapabilities(dev);
                inputDevice->m_name = libinput_device_get_name(dev);
                inputDevice->m_vendorId = libinput_device_get_id_vendor(dev);
                inputDevice->m_productId = libinput_device_get_id_product(dev);
                pluggedDevices.push_back(inputDevice);
                inputDevice->notifyPlugged();
                break;
            case LIBINPUT_EVENT_DEVICE_REMOVED:
                dev = libinput_event_get_device(ev);
                inputDevice = (LInputDevice*)libinput_device_get_user_data(dev);
                LVectorRemoveOneUnordered(pluggedDevices, inputDevice);
                unpluggedDevices.push_back(inputDevice);
                inputDevice->notifyUnplugged();
                inputDevice->m_nativeHandle = nullptr;
                break;
            default:
                break;
            }

            seat()->nativeInputEvent(ev);
            libinput_event_destroy(ev);
        }

        return 0;
    }

    static UInt32 backendGetId()
    {
        return LInputBackendLibinput;
    }

    static void *backendGetContextHandle()
    {
        return li;
    }

    static const std::vector<LInputDevice*> *backendGetDevices()
    {
        return &pluggedDevices;
    }

    static bool backendInitialize()
    {
        Int32 fd;
        libseatEnabled = seat()->imp()->initLibseat();

        ud = udev_new();

        if (!ud)
        {
            goto fail;
        }

        libinputInterface.open_restricted = &openRestricted;
        libinputInterface.close_restricted = &closeRestricted;
        li = libinput_udev_create_context(&libinputInterface, NULL, ud);

        if (!li)
            goto fail;

        if (libseatEnabled )
            libinput_udev_assign_seat(li, libseat_seat_name(seat()->libseatHandle()));
        else
            libinput_udev_assign_seat(li, "seat0");

        fd = libinput_get_fd(li);

        eventSource = wl_event_loop_add_fd(
            LCompositor::eventLoop(),
            fd,
            WL_EVENT_READABLE,
            &LInputBackend::processInput,
            (LSeat*)seat);
        return true;

    fail:
        backendUninitialize();
        return false;
    }

    static void backendUninitialize()
    {
        if (eventSource)
        {
            wl_event_source_remove(eventSource);
            eventSource = nullptr;
        }

        // Only delete devices, do not notify
        while (!pluggedDevices.empty())
        {
            delete pluggedDevices.back();
            pluggedDevices.pop_back();
        }

        while (!unpluggedDevices.empty())
        {
            delete unpluggedDevices.back();
            unpluggedDevices.pop_back();
        }

        if (li)
        {
            libinput_unref(li);
            li = nullptr;
        }

        if (ud)
        {
            udev_unref(ud);
            ud = nullptr;
        }
    }

    static void backendSuspend()
    {
        libinput_suspend(li);
    }

    static void backendResume()
    {
        if (libinput_resume(li) == -1)
            LLog::error("[Libinput Backend] Failed to resume libinput.");
    }

    static void backendForceUpdate()
    {
        processInput(0, 0, NULL);
    }

    static void backendSetLeds(UInt32 leds)
    {
        for (auto device : pluggedDevices)
            libinput_device_led_update((libinput_device*)device->m_nativeHandle, (libinput_led)leds);
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
    API.backendSetLeds          = &LInputBackend::backendSetLeds;
    API.backendForceUpdate      = &LInputBackend::backendForceUpdate;
    return &API;
}
