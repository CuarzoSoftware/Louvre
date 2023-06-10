#include <LInputBackend.h>
#include <LWayland.h>

#include <private/LCompositorPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LOutputPrivate.h>
#include <LLog.h>

#include <X11/Xlib.h>

#define BACKEND_NAME "X11_INPUT_BACKEND: "

using namespace Louvre;

KeySym k;
XEvent event;
float axisX = 0, axisY = 0;
bool axisEvent = false;
float axisStep = 7;

struct BACKEND_DATA
{
    Display *display;
    Window window;
    LSeat *seat;
    wl_event_source *source = nullptr;
};

struct OUTPUT_DATA
{
    Int32 backendId;
    Display *display;
    Window window;
};


int processInput(int, unsigned int, void *userData)
{
    LSeat *seat = (LSeat*)userData;
    LInputBackend::forceUpdate(seat);
    return 0;
}

void LInputBackend::forceUpdate(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;

    while(XPending(data->display))
    {
        XNextEvent(data->display, &event);

        if(event.type == MotionNotify)
        {
            seat->pointer()->pointerPosChangeEvent(event.xbutton.x, event.xbutton.y);
        }
        else if (event.type == ButtonPress)
        {
            if (event.xbutton.button == Button1)
                seat->pointer()->pointerButtonEvent(LPointer::Button::Left, LPointer::ButtonState::Pressed);
            else if (event.xbutton.button == Button2)
                seat->pointer()->pointerButtonEvent(LPointer::Button::Middle, LPointer::ButtonState::Pressed);
            else if (event.xbutton.button == Button3)
                seat->pointer()->pointerButtonEvent(LPointer::Button::Right, LPointer::ButtonState::Pressed);

            // Scroll Up
            else if (event.xbutton.button == Button4)
            {
                axisY = axisStep;
                axisEvent = true;
            }
            // Scroll Down
            else if (event.xbutton.button == Button5)
            {
                axisY = -axisStep;
                axisEvent = true;
            }
            // Scroll Left
            else if (event.xbutton.button == 6)
            {
                axisX = axisStep;
                axisEvent = true;
            }
            // Scroll Right
            else if (event.xbutton.button == 7)
            {
                axisX = -axisStep;
                axisEvent = true;
            }


        }
        else if (event.type == ButtonRelease)
        {
            if (event.xbutton.button == Button1)
                seat->pointer()->pointerButtonEvent(LPointer::Button::Left, LPointer::ButtonState::Released);
            else if (event.xbutton.button == Button2)
                seat->pointer()->pointerButtonEvent(LPointer::Button::Middle, LPointer::ButtonState::Released);
            else if (event.xbutton.button == Button3)
                seat->pointer()->pointerButtonEvent(LPointer::Button::Right, LPointer::ButtonState::Released);
            // Scroll Up
            else if (event.xbutton.button == Button4)
            {
                axisY = 0;
                axisEvent = true;
            }
            // Scroll Down
            else if (event.xbutton.button == Button5)
            {
                axisY = 0;
                axisEvent = true;
            }
            // Scroll Left
            else if (event.xbutton.button == 6)
            {
                axisX = 0;
                axisEvent = true;
            }
            // Scroll Right
            else if (event.xbutton.button == 7)
            {
                axisX = 0;
                axisEvent = true;
            }
        }
        else if (event.type == KeyPress)
        {
            xkb_state_update_key(seat->keyboard()->imp()->xkbKeymapState,
                                 event.xkey.keycode,
                                 XKB_KEY_DOWN);

            seat->keyboard()->keyEvent(event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_PRESSED);
            seat->keyboard()->imp()->updateModifiers();

        }
        else if (event.type == KeyRelease)
        {
            xkb_state_update_key(seat->keyboard()->imp()->xkbKeymapState,
                                 event.xkey.keycode,
                                 XKB_KEY_UP);

            seat->keyboard()->keyEvent(event.xkey.keycode - 8, WL_KEYBOARD_KEY_STATE_RELEASED);
            seat->keyboard()->imp()->updateModifiers();
        }
        if(axisEvent)
        {
            axisEvent = false;

            #if LOUVRE_WL_SEAT_VERSION >= 5
                seat->pointer()->pointerAxisEvent(axisX, axisY, WL_POINTER_AXIS_SOURCE_WHEEL);
            #else
                seat->pointer()->pointerAxisEvent(axisX, axisY);
            #endif
        }

        #if LOUVRE_DEBUG == 0
        if(event.type == FocusOut)
            XSetInputFocus(data->display, data->window, RevertToParent, CurrentTime);
        #endif

        ((LSeat*)seat)->backendNativeEvent((void*)&event);
    }

}

bool LInputBackend::initialize(const LSeat *seat)
{
    BACKEND_DATA *data = new BACKEND_DATA;
    data->seat = (LSeat*)seat;
    seat->imp()->inputBackendData = data;
    OUTPUT_DATA *outputData;
    int fd;

    if(seat->compositor()->outputManager()->outputs()->empty())
        goto failDep;
    else
        outputData = (OUTPUT_DATA*)seat->compositor()->outputManager()->outputs()->front()->imp()->graphicBackendData;

    if(outputData->backendId != 1)
        goto failDep;

    data->display = outputData->display;
    data->window = outputData->window;

    if(!data->display)
    {
        LLog::error("%sFailed to get X display.", BACKEND_NAME);
        goto fail;
    }

    //XSelectInput(data->display, data->window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | FocusChangeMask);

    fd = ConnectionNumber(data->display);

    data->source = LWayland::addFdListener(fd, (LSeat*)seat, &processInput);

    LLog::debug("X11 Input Backend initialzed.");
    return true;

    failDep:
    LLog::error("%sNeeds X11 graphic backend.", BACKEND_NAME);

    fail:
    delete data;
    seat->imp()->inputBackendData = nullptr;
    return false;

}

UInt32 LInputBackend::getCapabilities(const LSeat *seat)
{
    L_UNUSED(seat);
    return LSeat::Capabilities::Pointer | LSeat::Capabilities::Keyboard;
}

void *LInputBackend::getContextHandle(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    return data->display;
}

void LInputBackend::suspend(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    LWayland::removeFdListener(data->source);
    data->source = nullptr;
}

void LInputBackend::resume(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;
    data->source = LWayland::addFdListener(ConnectionNumber(data->display), (LSeat*)seat, &processInput);
}

void LInputBackend::uninitialize(const LSeat *seat)
{
    BACKEND_DATA *data = (BACKEND_DATA*)seat->imp()->inputBackendData;

    if(data->source)
        LWayland::removeFdListener(data->source);

    delete data;
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
