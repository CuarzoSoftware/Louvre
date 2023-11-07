#include <protocols/Wayland/private/GDataDeviceManagerPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::Wayland;

GDataDeviceManager::GDataDeviceManager
(
    LClient *client,
    const wl_interface *interface,
    Int32 version,
    UInt32 id,
    const void *implementation,
    wl_resource_destroy_func_t destroy
)
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation,
        destroy
    )
{
    m_imp = new GDataDeviceManagerPrivate();
    client->imp()->dataDeviceManagerGlobal = this;
}

GDataDeviceManager::~GDataDeviceManager()
{
    client()->imp()->dataDeviceManagerGlobal = nullptr;
    delete m_imp;
}
