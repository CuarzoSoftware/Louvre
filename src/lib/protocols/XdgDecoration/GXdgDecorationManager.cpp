#include <protocols/XdgDecoration/private/GXdgDecorationManagerPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::XdgDecoration;

GXdgDecorationManager::GXdgDecorationManager
(
    wl_client *client,
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
    LPRIVATE_INIT_UNIQUE(GXdgDecorationManager)
{
    this->client()->imp()->xdgDecorationManagerGlobals.push_back(this);
}

GXdgDecorationManager::~GXdgDecorationManager()
{
    LVectorRemoveOneUnordered(client()->imp()->xdgDecorationManagerGlobals, this);
}
