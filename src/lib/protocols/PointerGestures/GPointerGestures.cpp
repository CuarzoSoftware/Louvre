#include <protocols/PointerGestures/private/GPointerGesturesPrivate.h>
#include <protocols/PointerGestures/pointer-gestures-unstable-v1.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::PointerGestures;

GPointerGestures::GPointerGestures
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
    LPRIVATE_INIT_UNIQUE(GPointerGestures)
{
    this->client()->imp()->pointerGesturesGlobals.push_back(this);
}

GPointerGestures::~GPointerGestures()
{
    LVectorRemoveOneUnordered(client()->imp()->pointerGesturesGlobals, this);
}
