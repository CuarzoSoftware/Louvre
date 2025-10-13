#include <CZ/Louvre/Backends/DRM/LDRMBackend.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Louvre/Private/LSeatPrivate.h>

#include <CZ/Core/Events/CZPointerMoveEvent.h>
#include <CZ/Core/Events/CZPointerScrollEvent.h>
#include <CZ/Core/Events/CZPointerButtonEvent.h>
#include <CZ/Core/Events/CZPointerSwipeBeginEvent.h>
#include <CZ/Core/Events/CZPointerSwipeUpdateEvent.h>
#include <CZ/Core/Events/CZPointerSwipeEndEvent.h>
#include <CZ/Core/Events/CZPointerPinchBeginEvent.h>
#include <CZ/Core/Events/CZPointerPinchUpdateEvent.h>
#include <CZ/Core/Events/CZPointerPinchEndEvent.h>
#include <CZ/Core/Events/CZPointerHoldBeginEvent.h>
#include <CZ/Core/Events/CZPointerHoldEndEvent.h>
#include <CZ/Core/Events/CZKeyboardKeyEvent.h>
#include <CZ/Core/Events/CZTouchDownEvent.h>
#include <CZ/Core/Events/CZTouchUpEvent.h>
#include <CZ/Core/Events/CZTouchMoveEvent.h>
#include <CZ/Core/Events/CZTouchFrameEvent.h>
#include <CZ/Core/Events/CZTouchCancelEvent.h>

#include <CZ/Core/CZInputDevice.h>
#include <CZ/Core/CZCore.h>

using namespace CZ;

const static libinput_interface LibinputIface
{
    .open_restricted = [](const char *path, int flags, void *data)
    {
        auto &backend { *static_cast<LDRMBackend*>(data) };
        return backend.inputOpenDevice(path, flags);
    },
    .close_restricted = [](int fd, void *data)
    {
        auto &backend { *static_cast<LDRMBackend*>(data) };
        backend.inputCloseDevice(fd);
    }
};

static UInt32 DeviceCapabilities(libinput_device *dev)
{
    UInt32 caps { 0 };

    for (UInt32 cap = 0; cap <= LIBINPUT_DEVICE_CAP_SWITCH; cap++)
    {
        if (libinput_device_has_capability(dev, (libinput_device_capability)cap))
            caps |= (1 << cap);
    }

    return caps;
}

int LDRMBackend::inputOpenDevice(const char *path, int flags) noexcept
{
    if (m_libseatEnabled)
    {
        SeatDevice dev {};
        dev.id = seat()->openDevice(path, &dev.fd);

        if (dev.id < 0)
            return -1;

        m_inputSeatDevices.emplace_back(dev);
        return dev.fd;
    }
    else
        return open(path, flags);
}

void LDRMBackend::inputCloseDevice(int fd) noexcept
{
    if (m_libseatEnabled)
    {
        LDRMBackend::SeatDevice dev {};

        for (size_t i = 0; i < m_inputSeatDevices.size(); i++)
        {
            if (m_inputSeatDevices[i].fd == fd)
            {
                dev = m_inputSeatDevices[i];
                m_inputSeatDevices[i] = m_inputSeatDevices.back();
                m_inputSeatDevices.pop_back();
                break;
            }
        }

        if (dev.fd == -1)
            return;

        seat()->closeDevice(dev.id);
    }
    else
        close(fd);
}

bool LDRMBackend::inputInit() noexcept
{
    m_libinput = libinput_udev_create_context(&LibinputIface, this, m_udev);

    if (!m_libinput)
        return false;

    if (m_libseatEnabled)
        libinput_udev_assign_seat(m_libinput, libseat_seat_name(seat()->libseatHandle()));
    else
        libinput_udev_assign_seat(m_libinput, "seat0");

    m_libinputEventSource = CZEventSource::Make(
        libinput_get_fd(m_libinput), EPOLLIN, CZOwn::Borrow, [this](auto, auto) {
            inputDispatch();
        });

    return true;
}

void LDRMBackend::inputSuspend() noexcept
{
    libinput_suspend(m_libinput);
}

void LDRMBackend::inputResume() noexcept
{
    if (libinput_resume(m_libinput) == -1)
        iLog(CZError, CZLN, "Failed to resume");
}

void LDRMBackend::inputDispatch() noexcept
{
    const int ret { libinput_dispatch(m_libinput) };

    if (ret != 0)
    {
        iLog(CZError, CZLN, "Failed to dispatch events {}", strerror(-ret));
        return;
    }

    auto cz { CZCore::Get() };
    auto &seat { *CZ::seat() };

    while (auto *ev = libinput_get_event(m_libinput))
    {
        const auto eventType { libinput_event_get_type(ev) };
        auto *dev { libinput_event_get_device(ev) };

        switch (eventType)
        {
        case LIBINPUT_EVENT_POINTER_MOTION:
        {
            auto *nativeEvent { libinput_event_get_pointer_event(ev) };
            auto e { CZPointerMoveEvent() };
            e.delta.fX = libinput_event_pointer_get_dx(nativeEvent);
            e.delta.fY = libinput_event_pointer_get_dy(nativeEvent);
            e.deltaUnaccelerated.fX = libinput_event_pointer_get_dx_unaccelerated(nativeEvent);
            e.deltaUnaccelerated.fY = libinput_event_pointer_get_dy_unaccelerated(nativeEvent);
            e.ms = libinput_event_pointer_get_time(nativeEvent);
            e.us = libinput_event_pointer_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        {
            auto *nativeEvent { libinput_event_get_pointer_event(ev) };
            auto e { CZPointerMoveEvent() };
            SkScalar dx, dy;

            if (cursor() && cursor()->output())
            {
                dx = SkScalar(cursor()->output()->pos().x()) +
                     libinput_event_pointer_get_absolute_x_transformed(nativeEvent, cursor()->output()->size().width()) -
                     cursor()->pos().x();
                dy = SkScalar(cursor()->output()->pos().y()) +
                     libinput_event_pointer_get_absolute_y_transformed(nativeEvent, cursor()->output()->size().height()) -
                     cursor()->pos().y();
            }
            else
                dx = dy = 0.f;

            e.delta.fX = e.deltaUnaccelerated.fX = dx;
            e.delta.fY = e.deltaUnaccelerated.fY = dy;
            e.ms = libinput_event_pointer_get_time(nativeEvent);
            e.us = libinput_event_pointer_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            cz->sendEvent(e, seat);
            break;
        }
        // Legacy wheel scroll events
        case LIBINPUT_EVENT_POINTER_AXIS:
        {
            auto *nativeEvent { libinput_event_get_pointer_event(ev) };
            auto e { CZPointerScrollEvent() };
            e.ms = libinput_event_pointer_get_time(nativeEvent);
            e.us = libinput_event_pointer_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.hasX = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0;
            e.hasY = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0;
            e.relativeDirectionX = e.relativeDirectionY = libinput_device_config_scroll_get_natural_scroll_enabled(dev) == 0 ? CZPointerScrollEvent::Identical : CZPointerScrollEvent::Inverted;

            if (libinput_event_pointer_get_axis_source(nativeEvent) == LIBINPUT_POINTER_AXIS_SOURCE_WHEEL)
            {
                if (e.hasX)
                {
                    e.axes.fX = libinput_event_pointer_get_axis_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
                    e.axesDiscrete.fX = libinput_event_pointer_get_axis_value_discrete(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
                }

                if (e.hasY)
                {
                    e.axes.fY = libinput_event_pointer_get_axis_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
                    e.axesDiscrete.fY = libinput_event_pointer_get_axis_value_discrete(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
                }

                e.source = CZPointerScrollEvent::WheelLegacy;
                cz->sendEvent(e, seat);
            }
            else if (libinput_event_pointer_get_axis_source(nativeEvent) == LIBINPUT_POINTER_AXIS_SOURCE_WHEEL_TILT)
            {
                if (e.hasX)
                    e.axes.fX = libinput_event_pointer_get_axis_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);

                if (e.hasY)
                    e.axes.fY = libinput_event_pointer_get_axis_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

                e.source = CZPointerScrollEvent::WheelTilt;
                cz->sendEvent(e, seat);
            }
            break;
        }
        case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
        {
            auto *nativeEvent { libinput_event_get_pointer_event(ev) };
            auto e { CZPointerScrollEvent() };
            e.ms = libinput_event_pointer_get_time(nativeEvent);
            e.us = libinput_event_pointer_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.hasX = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0;
            e.hasY = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0;
            e.relativeDirectionX = e.relativeDirectionY = libinput_device_config_scroll_get_natural_scroll_enabled(dev) == 0 ? CZPointerScrollEvent::Identical : CZPointerScrollEvent::Inverted;

            if (e.hasX)
                e.axes.fX = libinput_event_pointer_get_scroll_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);

            if (e.hasY)
                e.axes.fY = libinput_event_pointer_get_scroll_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

            e.source = CZPointerScrollEvent::Finger;
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
        {
            auto *nativeEvent { libinput_event_get_pointer_event(ev) };
            auto e { CZPointerScrollEvent() };
            e.ms = libinput_event_pointer_get_time(nativeEvent);
            e.us = libinput_event_pointer_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.hasX = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0;
            e.hasY = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0;
            e.relativeDirectionX = e.relativeDirectionY = libinput_device_config_scroll_get_natural_scroll_enabled(dev) == 0 ? CZPointerScrollEvent::Identical : CZPointerScrollEvent::Inverted;

            if (e.hasX)
                e.axes.fX = libinput_event_pointer_get_scroll_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);

            if (e.hasY)
                e.axes.fY = libinput_event_pointer_get_scroll_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

            e.source = CZPointerScrollEvent::Continuous;
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
        {
            auto *nativeEvent { libinput_event_get_pointer_event(ev) };
            auto e { CZPointerScrollEvent() };
            e.ms = libinput_event_pointer_get_time(nativeEvent);
            e.us = libinput_event_pointer_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.hasX = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL) != 0;
            e.hasY = libinput_event_pointer_has_axis(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL) != 0;
            e.relativeDirectionX = e.relativeDirectionY = libinput_device_config_scroll_get_natural_scroll_enabled(dev) == 0 ? CZPointerScrollEvent::Identical : CZPointerScrollEvent::Inverted;

            if (e.hasX)
            {
                e.axes.fX = libinput_event_pointer_get_scroll_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
                e.axesDiscrete.fX = libinput_event_pointer_get_scroll_value_v120(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);
            }

            if (e.hasY)
            {
                e.axes.fY = libinput_event_pointer_get_scroll_value(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
                e.axesDiscrete.fY = libinput_event_pointer_get_scroll_value_v120(nativeEvent, LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);
            }

            e.source = CZPointerScrollEvent::Wheel;
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_POINTER_BUTTON:
        {
            auto *nativeEvent { libinput_event_get_pointer_event(ev) };
            auto e { CZPointerButtonEvent() };
            e.ms = libinput_event_pointer_get_time(nativeEvent);
            e.us = libinput_event_pointer_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.button = libinput_event_pointer_get_button(nativeEvent);
            e.pressed = libinput_event_pointer_get_button_state(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerSwipeBeginEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerSwipeUpdateEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            e.delta.fX = libinput_event_gesture_get_dx(nativeEvent);
            e.delta.fY = libinput_event_gesture_get_dy(nativeEvent);
            e.deltaUnaccelerated.fX = libinput_event_gesture_get_dx_unaccelerated(nativeEvent);
            e.deltaUnaccelerated.fY = libinput_event_gesture_get_dy_unaccelerated(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_SWIPE_END:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerSwipeEndEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            e.cancelled = libinput_event_gesture_get_cancelled(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerPinchBeginEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerPinchUpdateEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            e.delta.fX = libinput_event_gesture_get_dx(nativeEvent);
            e.delta.fY = libinput_event_gesture_get_dy(nativeEvent);
            e.deltaUnaccelerated.fX = libinput_event_gesture_get_dx_unaccelerated(nativeEvent);
            e.deltaUnaccelerated.fY = libinput_event_gesture_get_dy_unaccelerated(nativeEvent);
            e.scale = libinput_event_gesture_get_scale(nativeEvent);
            e.rotation = libinput_event_gesture_get_angle_delta(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_PINCH_END:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerPinchEndEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            e.cancelled = libinput_event_gesture_get_cancelled(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerHoldBeginEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_GESTURE_HOLD_END:
        {
            auto *nativeEvent { libinput_event_get_gesture_event(ev) };
            auto e { CZPointerHoldEndEvent() };
            e.ms = libinput_event_gesture_get_time(nativeEvent);
            e.us = libinput_event_gesture_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.fingers = libinput_event_gesture_get_finger_count(nativeEvent);
            e.cancelled = libinput_event_gesture_get_cancelled(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_KEYBOARD_KEY:
        {
            auto *nativeEvent { libinput_event_get_keyboard_event(ev) };
            auto e { CZKeyboardKeyEvent() };
            e.ms = libinput_event_keyboard_get_time(nativeEvent);
            e.us = libinput_event_keyboard_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.code = libinput_event_keyboard_get_key(nativeEvent);
            e.isPressed = libinput_event_keyboard_get_key_state(nativeEvent) == LIBINPUT_KEY_STATE_PRESSED;
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_TOUCH_DOWN:
        {
            auto *nativeEvent { libinput_event_get_touch_event(ev) };
            auto e { CZTouchDownEvent() };
            e.ms = libinput_event_touch_get_time(nativeEvent);
            e.us = libinput_event_touch_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.pos.fX = libinput_event_touch_get_x_transformed(nativeEvent, 1);
            e.pos.fY = libinput_event_touch_get_y_transformed(nativeEvent, 1);
            e.id = libinput_event_touch_get_seat_slot(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_TOUCH_MOTION:
        {
            auto *nativeEvent { libinput_event_get_touch_event(ev) };
            auto e { CZTouchMoveEvent() };
            e.ms = libinput_event_touch_get_time(nativeEvent);
            e.us = libinput_event_touch_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.pos.fX = libinput_event_touch_get_x_transformed(nativeEvent, 1);
            e.pos.fY = libinput_event_touch_get_y_transformed(nativeEvent, 1);
            e.id = libinput_event_touch_get_seat_slot(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_TOUCH_UP:
        {
            auto *nativeEvent { libinput_event_get_touch_event(ev) };
            auto e { CZTouchUpEvent() };
            e.ms = libinput_event_touch_get_time(nativeEvent);
            e.us = libinput_event_touch_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            e.id = libinput_event_touch_get_seat_slot(nativeEvent);
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_TOUCH_FRAME:
        {
            auto *nativeEvent { libinput_event_get_touch_event(ev) };
            auto e { CZTouchFrameEvent() };
            e.ms = libinput_event_touch_get_time(nativeEvent);
            e.us = libinput_event_touch_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_TOUCH_CANCEL:
        {
            auto *nativeEvent { libinput_event_get_touch_event(ev) };
            auto e { CZTouchCancelEvent() };
            e.ms = libinput_event_touch_get_time(nativeEvent);
            e.us = libinput_event_touch_get_time_usec(nativeEvent);
            e.device = *static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev));
            cz->sendEvent(e, seat);
            break;
        }
        case LIBINPUT_EVENT_DEVICE_ADDED:
        {
            auto inputDevice { m_inputDevices.emplace(
                CZInputDevice::Make(
                    DeviceCapabilities(dev),
                    libinput_device_get_name(dev),
                    libinput_device_get_id_vendor(dev),
                    libinput_device_get_id_product(dev),
                    CZInputDevice::NativeHandleType::Libinput,
                    (CZInputDevice::NativeHandle)dev)) };
            libinput_device_set_user_data(dev, (std::shared_ptr<CZInputDevice>*)&(*inputDevice.first));
            seat.inputDevicePlugged(*inputDevice.first);
            break;
        }
        case LIBINPUT_EVENT_DEVICE_REMOVED:
        {
            auto it { m_inputDevices.find(*static_cast<std::shared_ptr<CZInputDevice>*>(libinput_device_get_user_data(dev))) };
            assert(it != m_inputDevices.end());
            it->get()->nativeHandle = {};
            it->get()->nativeHandleType = CZInputDevice::NativeHandleType::None;
            seat.inputDeviceUnplugged(*it);
            m_inputDevices.erase(it);
            break;
        }
        default:
            break;
        }

        seat.nativeInputEvent(ev);
        libinput_event_destroy(ev);
    }
}

const std::set<std::shared_ptr<CZInputDevice>> &LDRMBackend::inputDevices() const noexcept
{
    return m_inputDevices;
}

void LDRMBackend::inputSetLeds(UInt32 leds) noexcept
{
    for (auto &device : m_inputDevices)
    {
        if (device->nativeHandle.libinput)
            libinput_device_led_update(device->nativeHandle.libinput, (libinput_led)leds);
    }
}

void LDRMBackend::inputForceUpdate() noexcept
{
    inputDispatch();
}
