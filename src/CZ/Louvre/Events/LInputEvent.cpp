#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Events/LInputEvent.h>

using namespace Louvre;

void LInputEvent::setDevice(LInputDevice *device) noexcept
{
    if (device)
        m_device = device;
    else
        m_device = &compositor()->imp()->fakeDevice;
}
