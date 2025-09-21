#include <LCompositor.h>
#include <LCursor.h>
#include <LIdleListener.h>
#include <LKeyboard.h>
#include <LLog.h>
#include <LOutput.h>
#include <LPointer.h>
#include <LSeat.h>

using namespace Louvre;

//! [configureOutputsRequest]
bool LSeat::configureOutputsRequest(
    LClient *client, const std::vector<OutputConfiguration> &configurations) {
  // All requests accepted by default (unsafe) see LCompositor::globalsFilter().
  L_UNUSED(client)

  for (const auto &conf : configurations) {
    conf.output.setPos(conf.pos);
    conf.output.setTransform(conf.transform);
    conf.output.setScale(conf.scale);
    conf.output.setMode(&conf.mode);

    if (conf.initialized)
      compositor()->addOutput(&conf.output);
    else
      compositor()->removeOutput(&conf.output);
  }

  // Revert changes if the configuration results in zero initialized outputs
  return !compositor()->outputs().empty();
}
//! [configureOutputsRequest]

//! [nativeInputEvent]
void LSeat::nativeInputEvent(void *event) { L_UNUSED(event); }
//! [nativeInputEvent]

//! [enabledChanged]
void LSeat::enabledChanged() {
  if (!enabled()) return;

  cursor()->setVisible(false);
  cursor()->setVisible(true);
  cursor()->move(1, 1);
  compositor()->repaintAllOutputs();
}
//! [enabledChanged]

//! [outputPlugged]
void LSeat::outputPlugged(LOutput *output) {
  // Probably a VR headset, meant to be leased by clients
  if (output->isNonDesktop()) {
    output->setLeasable(true);
    return;
  }

  output->setScale(output->dpi() >= 200 ? 2 : 1);

  if (compositor()->outputs().empty())
    output->setPos(0);
  else
    output->setPos(compositor()->outputs().back()->pos() +
                   LPoint(compositor()->outputs().back()->size().w(), 0));

  compositor()->addOutput(output);
  compositor()->repaintAllOutputs();
}
//! [outputPlugged]

//! [outputUnplugged]
void LSeat::outputUnplugged(LOutput *output) {
  compositor()->removeOutput(output);

  Int32 totalWidth{0};

  for (LOutput *o : compositor()->outputs()) {
    o->setPos(LPoint(totalWidth, 0));
    totalWidth += o->size().w();
  }

  compositor()->repaintAllOutputs();
}
//! [outputUnplugged]

//! [inputDevicePlugged]
void LSeat::inputDevicePlugged(LInputDevice *device) { L_UNUSED(device); }
//! [inputDevicePlugged]

//! [inputDeviceUnplugged]
void LSeat::inputDeviceUnplugged(LInputDevice *device) { L_UNUSED(device); }
//! [inputDeviceUnplugged]

//! [isIdleStateInhibited]
bool LSeat::isIdleStateInhibited() const {
  for (LSurface *surface : idleInhibitorSurfaces())
    if (surface->mapped() && !surface->outputs().empty()) return true;

  return false;
}
//! [isIdleStateInhibited]

//! [onIdleListenerTimeout]
void LSeat::onIdleListenerTimeout(const LIdleListener &listener) {
  if (isIdleStateInhibited()) listener.resetTimer();

  /* If the timer is not reset, the client will assume the user is idle. */
}
//! [onIdleListenerTimeout]

//! [onEvent]
void LSeat::onEvent(const LEvent &event) {
  L_UNUSED(event)

  /*
   * for (const LIdleListener *idleListener : idleListeners())
   *     idleListener->resetTimer();
   *
   * Resetting all timers each time an event occurs is not CPU-friendly,
   * as multiple events can be triggered in a single main loop iteration.
   * Instead, using the option below (setIsUserIdleHint()) is more efficient.
   */

  /*
   * Setting this flag to false indicates the user wasn't idle during
   * this main loop iteration. If that's the case, Louvre will reset
   * all timers only once at the end of the iteration.
   * The flag is automatically set to true again afterwards.
   */
  setIsUserIdleHint(false);
}
//! [onEvent]

//! [eventFilter]
bool LSeat::eventFilter(LEvent &event) {
  L_UNUSED(event);

  // Allows propagation of all events
  return true;
}
//! [eventFilter]
