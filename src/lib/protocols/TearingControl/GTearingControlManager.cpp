#include <protocols/TearingControl/private/GTearingControlManagerPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::GammaControl;

GTearingControlManager::GTearingControlManager
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
        ),
    LPRIVATE_INIT_UNIQUE(GTearingControlManager)
{
    client->imp()->tearingControlManagerGlobals.push_back(this);
}

GTearingControlManager::~GTearingControlManager()
{
    LVectorRemoveOneUnordered(client()->imp()->tearingControlManagerGlobals, this);
}
