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
    )
{
    m_imp = new GXdgDecorationManagerPrivate();
    this->client()->imp()->xdgDecorationManagerGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->xdgDecorationManagerGlobals.end());
}

GXdgDecorationManager::~GXdgDecorationManager()
{
    client()->imp()->xdgDecorationManagerGlobals.erase(imp()->clientLink);
    delete m_imp;
}
