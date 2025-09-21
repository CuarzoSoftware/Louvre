#include "Seat.h"

#include <LAnimation.h>
#include <LClient.h>
#include <LCursor.h>
#include <LInputDevice.h>
#include <LLog.h>
#include <LScene.h>
#include <LSessionLockManager.h>
#include <libinput.h>

#include "Global.h"
#include "Output.h"
#include "Surface.h"
#include "Toplevel.h"
#include "Workspace.h"

Seat::Seat(const void *params) : LSeat(params) {}

void Seat::enabledChanged() {
  if (!enabled()) {
    cursor()->setVisible(false);
    return;
  }

  configureInputDevices();

  // Damage all as a there may be missing pageflips
  for (Output *output : G::outputs()) {
    G::scene()->mainView()->damageAll(output);
    output->repaint();
  }

  // The HW cursor may have changed in another session
  if (cursor()) {
    cursor()->setVisible(false);
    cursor()->setVisible(true);
    cursor()->move(1, 1);
  }
}

void Seat::nativeInputEvent(void *event) {
  if (compositor()->inputBackendId() != LInputBackendLibinput ||
      sessionLockManager()->state() != LSessionLockManager::Unlocked)
    return;

  libinput_event *ev{(libinput_event *)event};
  Output *output{(Output *)cursor()->output()};

  if (!output) return;

  if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN) {
    libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

    if (libinput_event_gesture_get_finger_count(gev) > 2) {
      if (output->animatedFullscreenToplevel) return;

      output->showAllWorkspaces();
      output->workspacesAnimation.stop();
      output->doingFingerWorkspaceSwipe = true;
      output->workspacesPosX = output->workspacesContainer.nativePos().x();

      for (Output *o : G::outputs()) o->workspaces.front()->stealChildren();
    }
  }

  else if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE) {
    libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

    if (libinput_event_gesture_get_finger_count(gev) < 3) return;

    dx = libinput_event_gesture_get_dx(gev);

    if (dx > 50.f)
      dx = 50.f;
    else if (dx < -50.f)
      dx = -50.f;

    if (output->animatedFullscreenToplevel) return;

    output->workspacesAnimation.stop();
    output->showAllWorkspaces();

    Float32 offset = fabs(output->currentWorkspace->pos().x());
    Float32 weight = powf(1.f - offset / swipeMargin, 3.f);

    // Slow down swipping if there is no workspace in that direction
    if (output->workspacesPosX > 0.f)
      dx *= weight * 0.2f;
    else if (output->workspacesPosX <
             -output->workspaces.back()->nativePos().x())
      dx *= weight * 0.2f;

    output->workspacesPosX += dx;
    output->workspacesContainer.setPos(output->workspacesPosX, 0);

    for (Output *o : G::outputs()) {
      for (Workspace *ws : o->workspaces) {
        ws->clipChildren();
        ws->setVisible(
            LRect(ws->pos() + o->pos(), o->size()).intersects(o->rect()));
      }
    }

    output->repaint();
  }

  else if (libinput_event_get_type(ev) == LIBINPUT_EVENT_GESTURE_SWIPE_END) {
    libinput_event_gesture *gev = libinput_event_get_gesture_event(ev);

    if (libinput_event_gesture_get_finger_count(gev) > 2) {
      output->doingFingerWorkspaceSwipe = false;

      if (output->animatedFullscreenToplevel) return;

      Workspace *targetWorkspace = output->currentWorkspace;

      // Switch to right ws
      if (std::next(targetWorkspace->outputLink) != output->workspaces.end() &&
          (dx < -5.2 ||
           output->currentWorkspace->pos().x() + output->pos().x() <
               output->pos().x() - swipeMargin)) {
        targetWorkspace = *std::next(targetWorkspace->outputLink);
      }
      // Switch to left ws
      else if (targetWorkspace != output->workspaces.front() &&
               (dx > 5.2 ||
                output->currentWorkspace->pos().x() + output->pos().x() >
                    output->pos().x() + swipeMargin)) {
        targetWorkspace = *std::prev(targetWorkspace->outputLink);
      }

      output->setWorkspace(targetWorkspace, 550, 2.5f,
                           0.2f + (0.11f * fabs(dx)) / 50.f);
    }
  }
}

void Seat::outputUnplugged(LOutput *output) {
  compositor()->removeOutput(output);

  Int32 x = 0;

  for (Output *o : G::outputs()) {
    o->setPos(x);
    o->updateWorkspacesPos();
    o->repaint();
    x += o->size().w();
  }

  if (G::outputs().empty()) return;

  Output *firstOutput = G::outputs().front();

  std::list<Surface *> surfs = G::surfaces();

  for (Surface *s : surfs) {
    if (!s->outputUnplugHandled) {
      s->setPos(50 + (rand() % (firstOutput->size().w() / 4)),
                50 + (rand() % (firstOutput->size().h() / 4)));
      s->sendOutputEnterEvent(firstOutput);
      s->requestNextFrame(false);
      G::reparentWithSubsurfaces(s, &firstOutput->workspaces.front()->surfaces);

      if (s->toplevel()) {
        Toplevel *tl = (Toplevel *)s->toplevel();
        tl->configureSize(tl->prevRect.size());
        tl->configureState(LToplevelRole::Activated);
        tl->quickUnfullscreen = false;
        tl->surf()->client()->flush();
      }
    }
  }

  firstOutput->setWorkspace(firstOutput->workspaces.front(), 560, 4.f);
}

void Seat::inputDevicePlugged(LInputDevice *device) {
  configureInputDevice(device);
}

bool Seat::configureOutputsRequest(
    LClient *client, const std::vector<OutputConfiguration> &confs) {
  if (LSeat::configureOutputsRequest(client, confs)) {
    G::arrangeOutputs();
    return true;
  }

  return false;
}

void Seat::configureInputDevices() noexcept {
  for (LInputDevice *dev : inputDevices()) configureInputDevice(dev);
}

void Seat::configureInputDevice(LInputDevice *device) noexcept {
  if (compositor()->inputBackendId() != LInputBackendLibinput ||
      !device->nativeHandle())
    return;

  libinput_device *libinputDev{
      static_cast<libinput_device *>(device->nativeHandle())};

  // Disable while typing
  libinput_device_config_dwt_set_enabled(libinputDev,
                                         LIBINPUT_CONFIG_DWT_DISABLED);

  // Disable touchpad click areas
  libinput_device_config_click_set_method(
      libinputDev, LIBINPUT_CONFIG_CLICK_METHOD_CLICKFINGER);

  // Enable natural scrolling
  libinput_device_config_scroll_set_natural_scroll_enabled(libinputDev, 1);

  // Tap to click
  // libinput_device_config_tap_set_enabled(libinputDev,
  // LIBINPUT_CONFIG_TAP_ENABLED);
}
