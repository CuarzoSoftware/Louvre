#include <protocols/Wayland/private/GCompositorPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::Wayland;

GCompositor::GCompositor
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
    LPRIVATE_INIT_UNIQUE(GCompositor)
{
    client->imp()->compositorGlobals.push_back(this);
}

GCompositor::~GCompositor()
{
    LVectorRemoveOneUnordered(client()->imp()->compositorGlobals, this);
}
