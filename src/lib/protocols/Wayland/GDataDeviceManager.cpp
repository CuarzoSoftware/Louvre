#include <protocols/Wayland/private/GDataDeviceManagerPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::Wayland;

GDataDeviceManager::GDataDeviceManager
(
    LClient *client,
    const wl_interface *interface,
    Int32 version,
    UInt32 id,
    const void *implementation
)
    :LResource
    (
        client,
        interface,
        version,
        id,
        implementation
    ),
    LPRIVATE_INIT_UNIQUE(GDataDeviceManager)
{
    client->imp()->dataDeviceManagerGlobals.push_back(this);
}

GDataDeviceManager::~GDataDeviceManager()
{
    LVectorRemoveOneUnordered(client()->imp()->dataDeviceManagerGlobals, this);
}
