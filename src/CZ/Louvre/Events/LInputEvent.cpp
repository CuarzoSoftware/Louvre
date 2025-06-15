#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <LInputEvent.h>

using namespace Louvre;

void LInputEvent::setDevice(LInputDevice *device) noexcept
{
    if (device)
        m_device = device;
    else
        m_device = &compositor()->imp()->fakeDevice;
}
