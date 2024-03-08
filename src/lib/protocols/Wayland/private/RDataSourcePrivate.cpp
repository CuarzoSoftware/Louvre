#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/GSeat.h>
#include <LClipboard.h>
#include <LDND.h>
#include <LSeat.h>
#include <LClient.h>
#include <cstring>

void RDataSource::RDataSourcePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RDataSource::RDataSourcePrivate::offer(wl_client *client, wl_resource *resource, const char *mime_type)
{
    L_UNUSED(client);
    RDataSource *rDataSource { static_cast<RDataSource*>(wl_resource_get_user_data(resource)) };
    rDataSource->imp()->mimeTypes.emplace_back(mime_type);

    if (rDataSource == seat()->clipboard()->m_dataSource.get())
    {
        rDataSource->requestPersistentMimeType(rDataSource->imp()->mimeTypes.back());

        if (seat()->clipboard()->m_dataOffer.get())
            seat()->clipboard()->m_dataOffer.get()->offer(mime_type);
    }
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataSource::RDataSourcePrivate::set_actions(wl_client *client, wl_resource *resource, UInt32 dnd_actions)
{
    /* TODO
    L_UNUSED(client);

    if (dnd_actions > 8)
    {
        wl_resource_post_error(resource, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK, "Invalid DND action mask.");
        return;
    }

    RDataSource *rDataSource { static_cast<RDataSource*>(wl_resource_get_user_data(resource)) };

    if (rDataSource->usage() != RDataSource::DND)
    {
        wl_resource_post_error(resource, WL_DATA_SOURCE_ERROR_INVALID_SOURCE, "Source usage is not DND.");
        return;
    }

    if (rDataSource->actions() == dnd_actions)
        return;

    rDataSource->imp()->actions = dnd_actions;

    if (seat()->dndManager()->dstClient())
        for (GSeat *s : seat()->dndManager()->dstClient()->seatGlobals())
            if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered())
            {
                s->dataDeviceResource()->dataOffered()->dataOfferResource()->sourceActions(dnd_actions);
                s->dataDeviceResource()->dataOffered()->imp()->updateDNDAction();
            }
    */
}
#endif
