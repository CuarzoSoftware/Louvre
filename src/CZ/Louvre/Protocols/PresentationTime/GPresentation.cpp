#include <CZ/Louvre/Protocols/PresentationTime/presentation-time.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/PresentationTime/GPresentation.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::PresentationTime;

static const struct wp_presentation_interface imp
{
    .destroy = &GPresentation::destroy,
    .feedback = &GPresentation::feedback
};

void GPresentation::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GPresentation(client, version, id);
}

Int32 GPresentation::maxVersion() noexcept
{
    return LOUVRE_PRESENTATION_VERSION;
}

const wl_interface *GPresentation::interface() noexcept
{
    return &wp_presentation_interface;
}

GPresentation::GPresentation
(
    wl_client *client,
    Int32 version,
    UInt32 id
) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->presentationTimeGlobals.push_back(this);

    if (seat()->outputs().empty())
        clockId(CLOCK_MONOTONIC);
    else
        clockId(compositor()->imp()->graphicBackend->outputGetClock(seat()->outputs().front()));
}

GPresentation::~GPresentation() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->presentationTimeGlobals, this);
}

/******************** REQUESTS ********************/

void GPresentation::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GPresentation::feedback(wl_client */*client*/, wl_resource *resource, wl_resource *surface, UInt32 id) noexcept
{
    new RPresentationFeedback(static_cast<GPresentation*>(wl_resource_get_user_data(resource)),
                              static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface))->surface(),
                              id);
}

/******************** EVENTS ********************/

void GPresentation::clockId(UInt32 clockId) noexcept
{
    wp_presentation_send_clock_id(resource(), clockId);
}
