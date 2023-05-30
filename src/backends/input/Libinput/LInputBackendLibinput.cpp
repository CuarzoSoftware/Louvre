#include <LInputBackend.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <unordered_map>

#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LKeyboardPrivate.h>

using namespace Louvre;

struct BACKEND_DATA
{
    libinput *li = nullptr;
    udev *ud = nullptr;
    libinput_interface libinputInterface;
    LSeat *seat;
};

double x = 0, y = 0;
libinput_event *ev;
libinput_event_type eventType;
libinput_event_pointer *pointerEvent;
uint32_t button;
libinput_button_state state;
libinput_event_keyboard *keyEv;
libinput_key_state keyState;
int keyCode;
libinput_event_pointer *axisEvent;
UInt32 source;
std::unordered_map<int,int>devices;

int openRestricted(const char *path, int flags, void *data)
{
    L_UNUSED(flags);
    /*
    libseat *seat = (libseat*)data;
    int id,fd;
    id = libseat_open_device(seat, path, &fd);
    devices[fd] = id;
*/


    return open(path, flags);
}

void closeRestricted(int fd, void *data)
{
    return;
    //printf("Libinput close\n");
    libseat *seat = (libseat*)data;
    libseat_close_device(seat, devices[fd]);
}

int processInput(int, unsigned int, void *userData)
{
    LSeat *seat = (LSeat*)userData;
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;

    int ret = libinput_dispatch(data->li);

    if (ret != 0)
    {
        printf("Failed to dispatch libinput: %s\n", strerror(-ret));
        return 0;
    }

    while ((ev = libinput_get_event(data->li)) != NULL)
    {
        eventType = libinput_event_get_type(ev);

        if (eventType == LIBINPUT_EVENT_POINTER_MOTION)
        {
            if (!seat->cursor())
                goto skip;

            pointerEvent = libinput_event_get_pointer_event(ev);

            x = libinput_event_pointer_get_dx(pointerEvent);
            y = libinput_event_pointer_get_dy(pointerEvent);

            if (seat->pointer())
                seat->pointer()->pointerMoveEvent(x,y);

        }
        else if (eventType == LIBINPUT_EVENT_POINTER_BUTTON)
        {
            if (!seat->cursor())
                goto skip;

            pointerEvent = libinput_event_get_pointer_event(ev);
            button = libinput_event_pointer_get_button(pointerEvent);
            state = libinput_event_pointer_get_button_state(pointerEvent);

            if (seat->pointer())
                seat->pointer()->pointerButtonEvent((LPointer::Button)button,(LPointer::ButtonState)state);
        }
        else if (eventType == LIBINPUT_EVENT_KEYBOARD_KEY)
        {
            if (seat->keyboard())
            {
                keyEv = libinput_event_get_keyboard_event(ev);
                keyState = libinput_event_keyboard_get_key_state(keyEv);
                keyCode = libinput_event_keyboard_get_key(keyEv);

                xkb_state_update_key(seat->keyboard()->imp()->xkbKeymapState,
                                     keyCode+8,
                                     (xkb_key_direction)keyState);

                seat->keyboard()->keyEvent(keyCode, keyState);
                seat->keyboard()->imp()->updateModifiers();

                // CTRL + ALT + (F1, F2, ..., F10) : Cambia de TTY.
                if (keyCode >= KEY_F1 && keyCode <= KEY_F10 && seat->keyboard()->isModActive(XKB_MOD_NAME_ALT) && seat->keyboard()->isModActive(XKB_MOD_NAME_CTRL))
                {
                    seat->setTTY(keyCode - KEY_F1 + 1);
                    return 0;
                }

            }
        }
        else if (eventType == LIBINPUT_EVENT_POINTER_AXIS)
        {
            if (!seat->cursor())
                goto skip;

            if (seat->pointer())
            {
                axisEvent = libinput_event_get_pointer_event(ev);

                if (libinput_event_pointer_has_axis(axisEvent,LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL))
                    x = libinput_event_pointer_get_axis_value(axisEvent,LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL);

                if (libinput_event_pointer_has_axis(axisEvent,LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL))
                    y = libinput_event_pointer_get_axis_value(axisEvent,LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL);

            #if LOUVRE_SEAT_VERSION >= 5
                source = libinput_event_pointer_get_axis_source(axisEvent);
                seat->pointer()->pointerAxisEvent(x,y,source);
            #else
                seat->pointer()->pointerAxisEvent(x,y);
            #endif
            }
        }

        seat->backendNativeEvent(ev);
        skip:
        libinput_event_destroy(ev);
        libinput_dispatch(data->li);
    }
    return 0;
}

bool LInputBackend::initialize(const LSeat *seat)
{
    BACKEND_DATA *data = new BACKEND_DATA;
    data->seat = (LSeat*)seat;
    seat->imp()->inputBackendData = data;
    data->ud = udev_new();

    if (!data->ud)
        goto fail;

    data->libinputInterface.open_restricted = &openRestricted;
    data->libinputInterface.close_restricted = &closeRestricted;
    data->li = libinput_udev_create_context(&data->libinputInterface, seat->libseatHandle(), data->ud);

    if (!data->li)
        goto fail;

    libinput_udev_assign_seat(data->li, "seat0");//libseat_seat_name(seat->libseatHandle()));
    libinput_dispatch(data->li);
    LCompositor::addFdListener(libinput_get_fd(data->li), (LSeat*)seat, &processInput);

    return true;

    fail:
    delete data;
    seat->imp()->inputBackendData = nullptr;
    return false;
}

UInt32 LInputBackend::getCapabilities(const LSeat *seat)
{
    L_UNUSED(seat);
    return LSeat::InputCapabilities::Pointer | LSeat::InputCapabilities::Keyboard;
}

void *LInputBackend::getContextHandle(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    return data->li;
}

void LInputBackend::suspend(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    libinput_suspend(data->li);
}

void LInputBackend::forceUpdate(const LSeat *seat)
{
    processInput(0, 0, (LSeat*)seat);
}

void LInputBackend::resume(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    libinput_resume(data->li);
}

void LInputBackend::uninitialize(const LSeat *seat)
{
    L_UNUSED(seat);
}

LInputBackendInterface API;

extern "C" LInputBackendInterface *getAPI()
{
    API.initialize = &LInputBackend::initialize;
    API.uninitialize = &LInputBackend::uninitialize;
    API.getCapabilities = &LInputBackend::getCapabilities;
    API.getContextHandle = &LInputBackend::getContextHandle;
    API.suspend = &LInputBackend::suspend;
    API.forceUpdate = &LInputBackend::forceUpdate;
    API.resume = &LInputBackend::resume;
    return &API;
}
