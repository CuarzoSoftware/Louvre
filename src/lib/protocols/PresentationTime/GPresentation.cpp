#include <protocols/PresentationTime/private/GPresentationPrivate.h>
#include <protocols/PresentationTime/presentation-time.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <LSeat.h>

using namespace Louvre::Protocols::PresentationTime;

GPresentation::GPresentation
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
    LPRIVATE_INIT_UNIQUE(GPresentation)
{
    this->client()->imp()->presentationTimeGlobals.push_back(this);

    if (seat()->outputs().empty())
        clockId(CLOCK_MONOTONIC);
    else
        clockId(compositor()->imp()->graphicBackend->outputGetClock(seat()->outputs().front()));
}

GPresentation::~GPresentation()
{
    LVectorRemoveOneUnordered(client()->imp()->presentationTimeGlobals, this);
}

bool GPresentation::clockId(UInt32 clockId)
{
    wp_presentation_send_clock_id(resource(), clockId);
    return true;
}
