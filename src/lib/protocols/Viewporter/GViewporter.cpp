#include <protocols/Viewporter/private/GViewporterPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::Wayland;

GViewporter::GViewporter
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
    LPRIVATE_INIT_UNIQUE(GViewporter)
{
    client->imp()->viewporterGlobals.push_back(this);
}

GViewporter::~GViewporter()
{
    LVectorRemoveOneUnordered(client()->imp()->viewporterGlobals, this);
}
