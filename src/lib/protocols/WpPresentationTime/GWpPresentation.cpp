#include <protocols/WpPresentationTime/private/GWpPresentationPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::WpPresentationTime;

GWpPresentation::GWpPresentation
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
    LPRIVATE_INIT_UNIQUE(GWpPresentation)
{
    this->client()->imp()->wpPresentationTimeGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->wpPresentationTimeGlobals.end());
    clockId(CLOCK_MONOTONIC);
}

GWpPresentation::~GWpPresentation()
{
    client()->imp()->wpPresentationTimeGlobals.erase(imp()->clientLink);
}

bool GWpPresentation::clockId(UInt32 clockId)
{
    wp_presentation_send_clock_id(resource(), clockId);
    return true;
}
