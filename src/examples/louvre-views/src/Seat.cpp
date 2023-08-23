#include <LCursor.h>
#include <LScene.h>
#include <libinput.h>
#include <LAnimation.h>
#include <LLog.h>
#include "Seat.h"
#include "Global.h"
#include "Output.h"
#include "Workspace.h"

Seat::Seat(Params *params) : LSeat(params)
{
}

// This is called when the TTY session is restored
void Seat::seatEnabled()
{
    // Damage all as a there may be missing pageflips
    for (Output *output : G::outputs())
    {
        G::scene()->mainView()->damageAll(output);
        output->repaint();
    }

    // The HW cursor may have changed in another session
    if (cursor())
    {
        cursor()->setVisible(false);
        cursor()->setVisible(true);
    }
}

void Seat::backendNativeEvent(void *event)
{
    libinput_event *ev = (libinput_event*)event;
    Output *output = (Output*)cursor()->output();

    if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN)
    {
        libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

        if (libinput_event_gesture_get_finger_count(gev) > 2)
        {
            if (output->animatedFullscreenToplevel)
                return;

            output->swippingWorkspace = true;
            output->workspaceOffset = output->workspacesContainer->pos().x();

            for (Output *o : G::outputs())
                o->workspaces.front()->stealChildren();
        }
    }

    else if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE)
    {
        libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

        if (libinput_event_gesture_get_finger_count(gev) < 3)
            return;

        dx = libinput_event_gesture_get_dx(gev);

        if (dx > 50.f)
            dx = 50.f;
        else if (dx < -50.f)
            dx = -50.f;

        if (output->animatedFullscreenToplevel)
            return;

        Float32 offset = fabs(output->currentWorkspace->pos().x());
        Float32 weight = powf(1.f - offset/swipeMargin, 3.f);

        // Slow down swipping if there is no workspace in that direction
        if (output->workspaceOffset > 0.f)
            dx *= weight * 0.2f;
        else if (output->workspaceOffset < - output->workspaces.back()->nativePos().x())
            dx *= weight * 0.2f;

        output->workspaceOffset += dx;
        output->workspacesContainer->setPos(output->workspaceOffset, 0);

        for (Output *o : G::outputs())
        {
            for (Workspace *ws : o->workspaces)
            {
                ws->clipChildren();
                ws->setVisible(LRect(ws->pos() + o->pos(), o->size()).intersects(o->rect()));
            }
        }

        output->repaint();
    }

    else if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_END)
    {
        libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

        if (libinput_event_gesture_get_finger_count(gev) > 2)
        {
            output->swippingWorkspace = false;

            if (output->animatedFullscreenToplevel)
                return;

            Workspace *targetWorkspace = output->currentWorkspace;

            // Switch to right ws
            if (std::next(targetWorkspace->outputLink) != output->workspaces.end() &&
                (dx < - 5.2 || output->currentWorkspace->pos().x() + output->pos().x() < output->pos().x() - swipeMargin))
            {
                targetWorkspace = *std::next(targetWorkspace->outputLink);
            }
            // Switch to left ws
            else if (targetWorkspace != output->workspaces.front() &&
                    (  dx > 5.2 || output->currentWorkspace->pos().x() + output->pos().x() > output->pos().x() + swipeMargin))
            {
                targetWorkspace = *std::prev(targetWorkspace->outputLink);
            }

            output->setWorkspace(targetWorkspace, 600, 3.5f, 0.15f + (0.3f * fabs(dx)) / 50.f);
        }
    }
}
