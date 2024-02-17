#include <LInputDevice.h>

using namespace Louvre;

void LInputDevice::notifyPlugged()
{
    seat()->inputDevicePlugged(this);
}

void LInputDevice::notifyUnplugged()
{
    seat()->inputDeviceUnplugged(this);
}
