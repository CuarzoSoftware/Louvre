#include "Presentation.h"
#include "LLog.h"
#include "presentation-time.h"

#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>

#include <LCompositor.h>

struct wp_presentation_interface presentation_implementation =
{
    .destroy = &Extensions::PresentationTime::Presentation::destroy,
    .feedback = &Extensions::PresentationTime::Presentation::feedback
};

using namespace Louvre::Extensions::PresentationTime;

void Presentation::resource_destroy(wl_resource *resource)
{
    LClient *lClient = (LClient*)wl_resource_get_user_data(resource);
    lClient->imp()->presentationTimeResource = nullptr;
}

void Presentation::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void Presentation::feedback(wl_client *client, wl_resource *resource, wl_resource *surface, UInt32 id)
{
    L_UNUSED(client);
    L_UNUSED(resource);
    wl_resource *feedback = wl_resource_create(client,
                                               &wp_presentation_feedback_interface,
                                               1,
                                               id);

    wl_resource_set_implementation(feedback, NULL, NULL,
            NULL);

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(surface);
    lSurface->imp()->presentationFeedback.push_back(feedback);
}

void Presentation::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LCompositor *lCompositor = (LCompositor*)data;

    LClient *lClient = lCompositor->getClientFromNativeResource(client);

    if(!lClient)
        return;

    if(lClient->imp()->presentationTimeResource)
    {
        LLog::warning("Client already created a wp_presentation resource.");
        return;
    }

    lClient->imp()->presentationTimeResource = wl_resource_create(client,
                                                                  &wp_presentation_interface,
                                                                  version,
                                                                  id);

    wl_resource_set_implementation(lClient->imp()->presentationTimeResource,
                                   &presentation_implementation,
                                   lClient,
                                   &Presentation::resource_destroy);

    wp_presentation_send_clock_id(lClient->imp()->presentationTimeResource,
                                  CLOCK_MONOTONIC);
}
