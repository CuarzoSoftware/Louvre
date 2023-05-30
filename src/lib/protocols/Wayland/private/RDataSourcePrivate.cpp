#include "RDataSourcePrivate.h"
#include <LClient.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <cstring>
#include <private/LDataSourcePrivate.h>

void RDataSource::RDataSourcePrivate::resource_destroy(wl_resource *resource)
{
    RDataSource *lRDataSource = (RDataSource*)wl_resource_get_user_data(resource);
    delete lRDataSource;
}

void RDataSource::RDataSourcePrivate::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RDataSource::RDataSourcePrivate::offer(wl_client *, wl_resource *resource, const char *mime_type)
{
    LDataSource::LSource source;
    int len = strlen(mime_type)+1;

    source.mimeType = new char[len];
    memcpy(source.mimeType,mime_type,len);

    RDataSource *lRDataSource = (RDataSource*)wl_resource_get_user_data(resource);
    source.tmp = tmpfile();
    lRDataSource->dataSource()->imp()->sources.push_back(source);
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void RDataSource::RDataSourcePrivate::set_actions(wl_client *, wl_resource *resource, UInt32 dnd_actions)
{
    if (dnd_actions > 8)
    {
        wl_resource_post_error(resource,WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,"Invalid DND action mask.");
        return;
    }

    RDataSource *lRDataSource = (RDataSource*)wl_resource_get_user_data(resource);
    lRDataSource->dataSource()->imp()->dndActions = dnd_actions;
}
#endif
