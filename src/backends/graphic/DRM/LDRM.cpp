#include <LCompositor.h>
#include <cstring>

#include "LDRM.h"

using namespace Louvre;

LDRM::LDRM(LCompositor *compositor)
{
    m_compositor = compositor;
}

LDRM::~LDRM()
{
    if(m_udev)
        udev_unref(m_udev);

    destroyDevices();
}

bool LDRM::initialize()
{
    m_udev = udev_new();

    if(!m_udev)
    {
        LLog::error("%s Failed to get udev.", LBACKEND_NAME);
        return false;
    }

    updateDevices();

    if(devices().empty())
    {
        LLog::error("%s No devices avaliable.", LBACKEND_NAME);
        return false;
    }

    return true;
}

void LDRM::updateDevices()
{
    udev_enumerate *enumerate;
    udev_list_entry *devices;
    udev_device *device;
    udev_list_entry *entry;

    enumerate = udev_enumerate_new(udevHandle());
    udev_enumerate_add_match_subsystem(enumerate, "drm");
    udev_enumerate_add_match_property(enumerate, "DEVTYPE", "drm_minor");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    // Loop GPUs
    udev_list_entry_foreach(entry, devices)
    {
        const char *path = udev_list_entry_get_name(entry);

        device = udev_device_new_from_syspath(udevHandle(), path);

        // E.g /dev/dri/card0
        const char *name = udev_device_get_property_value(device, "DEVNAME");

        // Check if in list
        LDRMDevice *ldevice = findDevice(name);

        // If not in list
        if(!ldevice)
        {
            LLog::debug("%s Testing device %s.", LBACKEND_NAME, name);

            ldevice = new LDRMDevice(this, device);

            if(ldevice->valid())
                this->devices().push_back(ldevice);
            else
                delete ldevice;
        }
        else
        {
            udev_device_unref(device);
            ldevice->update();
        }

    }

    /* TODO: check GPU hotplug */

    udev_enumerate_unref(enumerate);

}

void LDRM::destroyDevices()
{
    LDRMDevice *d;

    while(!devices().empty())
    {
        d = devices().back();
        delete d;
        devices().pop_back();
    }
}

LDRMDevice *LDRM::findDevice(const char *name)
{
    for(LDRMDevice *d : devices())
    {
        if(strcmp(d->name(),name) == 0)
            return d;
    }
    return nullptr;
}

LDRM::LDRMState LDRM::state() const
{
    return m_state;
}

LCompositor *LDRM::compositor() const
{
    return m_compositor;
}

udev *LDRM::udevHandle() const
{
    return m_udev;
}

list<LDRMDevice *> &LDRM::devices()
{
    return m_devices;
}

list<LOutput *> &LDRM::outputs()
{
    return m_outputs;
}
