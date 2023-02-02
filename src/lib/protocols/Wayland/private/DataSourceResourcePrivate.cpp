#include "DataSourceResourcePrivate.h"
#include <LClient.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <cstring>
#include <private/LDataSourcePrivate.h>

void DataSourceResource::DataSourceResourcePrivate::resource_destroy(wl_resource *resource)
{
    DataSourceResource *lDataSourceResource = (DataSourceResource*)wl_resource_get_user_data(resource);
    delete lDataSourceResource;
}

void DataSourceResource::DataSourceResourcePrivate::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void DataSourceResource::DataSourceResourcePrivate::offer(wl_client *, wl_resource *resource, const char *mime_type)
{
    LDataSource::LSource source;
    int len = strlen(mime_type)+1;

    source.mimeType = new char[len];
    memcpy(source.mimeType,mime_type,len);

    DataSourceResource *lDataSourceResource = (DataSourceResource*)wl_resource_get_user_data(resource);
    source.tmp = tmpfile();
    lDataSourceResource->dataSource()->imp()->sources.push_back(source);
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void DataSourceResource::DataSourceResourcePrivate::set_actions(wl_client *, wl_resource *resource, UInt32 dnd_actions)
{
    if(dnd_actions > 8)
    {
        wl_resource_post_error(resource,WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,"Invalid DND action mask.");
        return;
    }

    DataSourceResource *lDataSourceResource = (DataSourceResource*)wl_resource_get_user_data(resource);
    lDataSourceResource->dataSource()->imp()->dndActions = dnd_actions;

}
#endif
