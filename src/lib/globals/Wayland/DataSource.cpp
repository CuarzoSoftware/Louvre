#include <globals/Wayland/DataSource.h>

#include <private/LDataSourcePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LDataOfferPrivate.h>
#include <private/LDataDevicePrivate.h>
#include <private/LDNDManagerPrivate.h>

#include <LClient.h>
#include <LSeat.h>

#include <string.h>
#include <unistd.h>

using namespace Louvre::Globals;

void DataSource::resource_destroy(wl_resource *resource)
{
    LDataSource *lDataSource = (LDataSource*)wl_resource_get_user_data(resource);

    // Check if being used by a Drag & Drop
    if(lDataSource == lDataSource->client()->seat()->dndManager()->source())
    {
        lDataSource->client()->seat()->dndManager()->cancel();
    }

    // Check if used by clipboard
    if(lDataSource != lDataSource->client()->compositor()->seat()->dataSelection())
    {
        lDataSource->imp()->remove();
        delete lDataSource;
    }
    else
    {
        lDataSource->imp()->resource = nullptr;
    }

}

void DataSource::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void DataSource::offer(wl_client *, wl_resource *resource, const char *mime_type)
{
    LDataSource::LSource source;
    int len = strlen(mime_type)+1;

    source.mimeType = new char[len];
    memcpy(source.mimeType,mime_type,len);

    LDataSource *lDataSource = (LDataSource*)wl_resource_get_user_data(resource);
    source.tmp = tmpfile();
    lDataSource->imp()->sources.push_back(source);
}

#if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3
void DataSource::set_actions(wl_client *, wl_resource *resource, UInt32 dnd_actions)
{
    if(dnd_actions > 8)
    {
        wl_resource_post_error(resource,WL_DATA_SOURCE_ERROR_INVALID_ACTION_MASK,"Invalid DND action mask.");
        return;
    }

    LDataSource *lDataSource = (LDataSource*)wl_resource_get_user_data(resource);
    lDataSource->imp()->dndActions = dnd_actions;

}
#endif
