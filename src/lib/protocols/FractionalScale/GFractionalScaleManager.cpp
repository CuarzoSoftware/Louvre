#include <protocols/FractionalScale/private/GFractionalScaleManagerPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::FractionalScale;

GFractionalScaleManager::GFractionalScaleManager
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
    LPRIVATE_INIT_UNIQUE(GFractionalScaleManager)
{
    this->client()->imp()->fractionalScaleManagerGlobals.push_back(this);
}

GFractionalScaleManager::~GFractionalScaleManager()
{
    LVectorRemoveOneUnordered(client()->imp()->fractionalScaleManagerGlobals, this);
}
