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
    libinput_event *ev = (libinput_event*)event;
    Output *output = (Output*)cursor()->output();

    if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN)
    {
        libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

        if (libinput_event_gesture_get_finger_count(gev) == 3)
        {
            output->swippingWorkspace = true;
            output->workspaceAnim->stop();
            output->workspaceOffset = output->workspacesContainer->pos().x();
            output->workspaces.front()->stealChildren();
        }
    }

    else if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE)
    {
        libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

        if (libinput_event_gesture_get_finger_count(gev) != 3)
            return;

        output->workspaceAnim->stop();
        dx = libinput_event_gesture_get_dx(gev) * 0.2f;
        Float32 maxOffset = 128.f;
        Float32 offset = fabs(output->currentWorkspace->pos().x() - output->pos().x());
        Float32 weight = powf(1.f - offset/maxOffset, 3.f);

        if (dx < 0.f && output->currentWorkspace->pos().x() < output->pos().x())
            dx *= weight;

        else if (dx > 0.f && output->currentWorkspace->pos().x() > output->pos().x())
            dx *= weight;

        output->workspaceOffset += dx;
        output->workspacesContainer->setPos(output->workspaceOffset, 0);

        for (Workspace *workspace : output->workspaces)
            workspace->clipChildren();

        output->repaint();
    }

    else if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_END)
    {
        libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

        if (libinput_event_gesture_get_finger_count(gev) == 3)
        {
            output->swippingWorkspace = false;

            Workspace *targetWorkspace = output->currentWorkspace;

            if (fabs(dx) > 0.1f)
            {
                // Switch to right ws
                if (output->currentWorkspace->pos().x() < output->pos().x() - 16
                    && dx < 0.f
                    && std::next(targetWorkspace->outputLink) != output->workspaces.end())
                {
                    targetWorkspace = *std::next(targetWorkspace->outputLink);
                }
                // Switch to left ws
                else if (output->currentWorkspace->pos().x() > output->pos().x() + 16
                           && dx > 0.f
                           && targetWorkspace != output->workspaces.front())
                {
                    targetWorkspace = *std::prev(targetWorkspace->outputLink);
                }
            }

            Float32 easing = fabs(dx);

            if (easing > 16.f)
                easing = 1.0f;
            else
                easing = 2.f - easing/16.f;

            Int32 ms = 480;

            if (targetWorkspace == output->currentWorkspace)
                ms = 250;

            output->setWorkspace(targetWorkspace, ms, 2.5f, (2.f - easing) * 0.65f);
        }
    }
}
