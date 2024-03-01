#include <private/LResourcePrivate.h>
#include <LCompositor.h>
#include <LClient.h>

using namespace Louvre;

LResource::LResource(wl_resource *resource) :
    m_client(compositor()->getClientFromNativeResource(wl_resource_get_client(resource))),
    m_resource(resource)
{}

LResource::LResource(wl_client *client,
                     const wl_interface *interface,
                     Int32 version,
                     UInt32 id,
                     const void *implementation):
    m_client(compositor()->getClientFromNativeResource(client)),
    m_resource(wl_resource_create(client, interface, version, id))
{
    wl_resource_set_implementation(m_resource, implementation, this, [](wl_resource *res)
    {
        LResource *resource = (LResource*)wl_resource_get_user_data(res);
        resource->m_destroyed = true;
        delete resource;
    });
}

LResource::LResource(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation) :
    m_client(client),
    m_resource(wl_resource_create(client->client(), interface, version, id))

{
    wl_resource_set_implementation(m_resource, implementation, this, [](wl_resource *res)
    {
        LResource *resource = (LResource*)wl_resource_get_user_data(res);
        resource->m_destroyed = true;
        delete resource;
    });
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
