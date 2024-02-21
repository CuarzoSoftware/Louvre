#include <protocols/Wayland/private/GSubcompositorPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::Wayland;

GSubcompositor::GSubcompositor
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
    LPRIVATE_INIT_UNIQUE(GSubcompositor)
{
    this->client()->imp()->subcompositorGlobals.push_back(this);
}

GSubcompositor::~GSubcompositor()
{
    LVectorRemoveOneUnordered(client()->imp()->subcompositorGlobals, this);
}
