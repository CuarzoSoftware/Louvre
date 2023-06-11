#include <protocols/Wayland/private/RDataSourcePrivate.h>
#include <private/LDataSourcePrivate.h>
#include <LSeat.h>
#include <cstring>

void RDataSource::RDataSourcePrivate::resource_destroy(wl_resource *resource)
{
    RDataSource *rDataSource = (RDataSource*)wl_resource_get_user_data(resource);
    delete rDataSource;
}

void RDataSource::RDataSourcePrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RDataSource::RDataSourcePrivate::offer(wl_client *client, wl_resource *resource, const char *mime_type)
{
    L_UNUSED(client);
    LDataSource::LSource source;
    int len = strlen(mime_type)+1;
    source.mimeType = new char[len];
    memcpy(source.mimeType, mime_type, len);

    RDataSource *rDataSource = (RDataSource*)wl_resource_get_user_data(resource);
    source.tmp = tmpfile();
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
    rDataSource->dataSource()->imp()->dndActions = dnd_actions;
}
#endif
