#include <protocols/XdgDecoration/private/GXdgDecorationManagerPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::XdgDecoration;

GXdgDecorationManager::GXdgDecorationManager
(
    wl_client *client,
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
    LPRIVATE_INIT_UNIQUE(GXdgDecorationManager)
{
    this->client()->imp()->xdgDecorationManagerGlobals.push_back(this);
}

GXdgDecorationManager::~GXdgDecorationManager()
{
    LVectorRemoveOneUnordered(client()->imp()->xdgDecorationManagerGlobals, this);
}
