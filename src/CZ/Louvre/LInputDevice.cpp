#include <CZ/Louvre/LInputDevice.h>
#include <CZ/Louvre/LCompositor.h>

using namespace Louvre;

void LInputDevice::notifyPlugged()
{
    compositor()->seat()->inputDevicePlugged(this);
}

void LInputDevice::notifyUnplugged()
{
    compositor()->seat()->inputDeviceUnplugged(this);
}
