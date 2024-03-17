#include <LInputDevice.h>

using namespace Louvre;

void LInputDevice::notifyPlugged()
{
    LCompositor::compositor()->seat()->inputDevicePlugged(this);
}

void LInputDevice::notifyUnplugged()
{
    LCompositor::compositor()->seat()->inputDeviceUnplugged(this);
}
