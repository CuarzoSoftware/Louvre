#include <protocols/Wayland/private/GCompositorPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::Wayland;

GCompositor::GCompositor
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
    m_imp = new GCompositorPrivate();
    client->imp()->compositorGlobals.push_back(this);
    imp()->clientLink = std::prev(client->imp()->compositorGlobals.end());
}

GCompositor::~GCompositor()
{
    client()->imp()->compositorGlobals.erase(imp()->clientLink);
    delete m_imp;
}
