#include <protocols/WpPresentationTime/private/GWpPresentationPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>
#include <private/LClientPrivate.h>

using namespace Louvre::Protocols::WpPresentationTime;

GWpPresentation::GWpPresentation
(
    LCompositor *compositor,
    wl_client *client,
    const wl_interface *interface,
    Int32 version,
    UInt32 id,
    const void *implementation,
    wl_resource_destroy_func_t destroy
)
    :LResource
    (
        compositor,
        client,
        interface,
        version,
        id,
        implementation,
        destroy
    )
{
    m_imp = new GWpPresentationPrivate();
    this->client()->imp()->wpPresentationTimeGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->wpPresentationTimeGlobals.end());
    clock_id(CLOCK_MONOTONIC);
}

GWpPresentation::~GWpPresentation()
{
    client()->imp()->wpPresentationTimeGlobals.erase(imp()->clientLink);
    delete m_imp;
}

bool GWpPresentation::clock_id(UInt32 clockId) const
{
    wp_presentation_send_clock_id(resource(), clockId);
    return true;
}
