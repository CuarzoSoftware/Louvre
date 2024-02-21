#include <protocols/WpPresentationTime/private/GWpPresentationPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <LSeat.h>

using namespace Louvre::Protocols::WpPresentationTime;

GWpPresentation::GWpPresentation
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
    LPRIVATE_INIT_UNIQUE(GWpPresentation)
{
    this->client()->imp()->wpPresentationTimeGlobals.push_back(this);

    if (seat()->outputs().empty())
        clockId(CLOCK_MONOTONIC);
    else
        clockId(compositor()->imp()->graphicBackend->outputGetClock(seat()->outputs().front()));
}

GWpPresentation::~GWpPresentation()
{
    LVectorRemoveOneUnordered(client()->imp()->wpPresentationTimeGlobals, this);
}

bool GWpPresentation::clockId(UInt32 clockId)
{
    wp_presentation_send_clock_id(resource(), clockId);
    return true;
}
