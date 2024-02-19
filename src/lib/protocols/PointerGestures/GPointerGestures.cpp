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
    LPRIVATE_INIT_UNIQUE(GPointerGestures)
{
    this->client()->imp()->pointerGesturesGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->pointerGesturesGlobals.end());
}

GPointerGestures::~GPointerGestures()
{
    client()->imp()->pointerGesturesGlobals.erase(imp()->clientLink);
}
