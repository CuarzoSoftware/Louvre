#include <LCursor.h>
#include <LScene.h>
#include <libinput.h>
#include <LLog.h>
#include "Seat.h"
#include "Global.h"
#include "Output.h"

Seat::Seat(Params *params) : LSeat(params) {}

// This is called the TTY session is restored
void Seat::seatEnabled()
{
    // Damage all as a there may be missing pageflips
    for (Output *output : G::outputs())
        G::scene()->mainView()->damageAll(output);

    // The HW cursor may have changed in another session
    if (cursor())
    {
        cursor()->setVisible(false);
        cursor()->setVisible(true);
    }
}

void Seat::backendNativeEvent(void *event)
{
    /*
    libinput_event *ev = (libinput_event*)event;

    if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE)
    {
        libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

        for (Output *output : G::outputs())
        {
            output->setPos(output->pos() + LPointF(
                                               (Float32)libinput_event_gesture_get_dx(gev),
                                               (Float32)libinput_event_gesture_get_dy(gev)));
            output->repaint();
        }
    }
    */
}
