#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <protocols/Wayland/RDataOffer.h>
#include <protocols/Wayland/RDataDevice.h>
#include <protocols/Wayland/GSeat.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataSourcePrivate.h>
#include <private/LDataDevicePrivate.h>
#include <LDNDManager.h>
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
    LDataSource::LSource source;
    source.mimeType = strdup(mime_type);
    RDataSource *rDataSource = (RDataSource*)wl_resource_get_user_data(resource);
    source.tmp = NULL;
    rDataSource->dataSource()->imp()->sources.push_back(source);
}

#if LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataSource::RDataSourcePrivate::set_actions(wl_client *client, wl_resource *resource, UInt32 dnd_actions)
{
    L_UNUSED(client);

    if (dnd_actions > 8)
    {
        wl_resource_post_error(resource, WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK, "Invalid DND action mask.");
        return;
    }

    RDataSource *rDataSource = (RDataSource*)wl_resource_get_user_data(resource);

    if (rDataSource->dataSource()->imp()->dndActions == dnd_actions)
        return;

    rDataSource->dataSource()->imp()->dndActions = dnd_actions;

    if (seat()->dndManager()->dstClient())
        for (GSeat *s : seat()->dndManager()->dstClient()->seatGlobals())
            if (s->dataDeviceResource() && s->dataDeviceResource()->dataOffered())
            {
                s->dataDeviceResource()->dataOffered()->dataOfferResource()->sourceActions(dnd_actions);
                s->dataDeviceResource()->dataOffered()->imp()->updateDNDAction();
            }

}
#endif
