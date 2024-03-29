#include <protocols/GammaControl/private/GGammaControlManagerPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::GammaControl;

GGammaControlManager::GGammaControlManager
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
    LPRIVATE_INIT_UNIQUE(GGammaControlManager)
{
    client->imp()->gammaControlManagerGlobals.push_back(this);
}

GGammaControlManager::~GGammaControlManager()
{
    LVectorRemoveOneUnordered(client()->imp()->gammaControlManagerGlobals, this);
}
