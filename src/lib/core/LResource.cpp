#include <private/LResourcePrivate.h>
#include <LCompositor.h>
#include <LClient.h>

using namespace Louvre;

LResource::LResource(wl_resource *resource) : LPRIVATE_INIT_UNIQUE(LResource)
{
    imp()->resource = resource;
    imp()->client = LCompositor::compositor()->getClientFromNativeResource(wl_resource_get_client(resource));
}

LResource::LResource(wl_client *client,
                     const wl_interface *interface,
                     Int32 version,
                     UInt32 id,
                     const void *implementation,
                     wl_resource_destroy_func_t destroy) :
    LPRIVATE_INIT_UNIQUE(LResource)
{
    imp()->resource = wl_resource_create(client, interface, version, id);
    imp()->client = compositor()->getClientFromNativeResource(client);
    wl_resource_set_implementation(imp()->resource, implementation, this, destroy);
}

LResource::LResource(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation, wl_resource_destroy_func_t destroy) :
    LPRIVATE_INIT_UNIQUE(LResource)
{
    imp()->resource = wl_resource_create(client->client(), interface, version, id);
    imp()->client = client;
    wl_resource_set_implementation(imp()->resource, implementation, this, destroy);
}

LResource::~LResource() {}

wl_resource *LResource::resource() const
{
    return imp()->resource;
}

LClient *LResource::client() const
{
    return imp()->client;
}

Int32 LResource::version() const
{
    return wl_resource_get_version(resource());
}

UInt32 LResource::id() const
{
    return wl_resource_get_id(resource());
}

void LResource::destroy()
{
    wl_resource_destroy(resource());
}
