#include <CZ/Louvre/Protocols/PresentationTime/presentation-time.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/PresentationTime/GPresentation.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PresentationTime;

static const struct wp_presentation_interface imp
{
    .destroy = &GPresentation::destroy,
    .feedback = &GPresentation::feedback
};

LGLOBAL_INTERFACE_IMP(GPresentation, LOUVRE_PRESENTATION_VERSION, wp_presentation_interface)

bool GPresentation::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.Presentation)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.Presentation;
    return true;
}

GPresentation::GPresentation
(
    wl_client *client,
    Int32 version,
    UInt32 id
)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->presentationTimeGlobals.push_back(this);
    clockId(compositor()->backend()->presentationClock());
}

GPresentation::~GPresentation() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->presentationTimeGlobals, this);
}

/******************** REQUESTS ********************/

void GPresentation::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GPresentation::feedback(wl_client */*client*/, wl_resource *resource, wl_resource *surface, UInt32 id) noexcept
{
    new RPresentationFeedback(static_cast<GPresentation*>(wl_resource_get_user_data(resource)),
                              static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface))->surface(),
                              id);
}

/******************** EVENTS ********************/

void GPresentation::clockId(UInt32 clockId) noexcept
{
    wp_presentation_send_clock_id(resource(), clockId);
}
