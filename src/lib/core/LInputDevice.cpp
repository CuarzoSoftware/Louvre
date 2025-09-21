#include <LCompositor.h>
#include <LInputDevice.h>

using namespace Louvre;

void LInputDevice::notifyPlugged() {
  compositor()->seat()->inputDevicePlugged(this);
}

void LInputDevice::notifyUnplugged() {
  compositor()->seat()->inputDeviceUnplugged(this);
}
