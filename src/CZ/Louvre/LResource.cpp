#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LResource.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;

LResource::LResource(wl_client *client,
                     const wl_interface *interface,
                     Int32 version,
                     UInt32 id,
                     const void *implementation) noexcept :
    m_client(compositor()->getClientFromNativeResource(client)),
    m_resource(wl_resource_create(client, interface, version, id))
{
    m_client->imp()->resources.push_back(this);
    m_clientLink = std::prev(m_client->imp()->resources.end());
    wl_resource_set_implementation(m_resource, implementation, this, [](wl_resource *res)
    {
        LResource *resource = (LResource*)wl_resource_get_user_data(res);
        delete resource;
    });
}

LResource::LResource(LClient *client, const wl_interface *interface, Int32 version, UInt32 id, const void *implementation) noexcept :
    m_client(client),
    m_resource(wl_resource_create(client->client(), interface, version, id))

{
    m_client->imp()->resources.push_back(this);
    m_clientLink = std::prev(m_client->imp()->resources.end());
    wl_resource_set_implementation(m_resource, implementation, this, [](wl_resource *res)
    {
        LResource *resource = (LResource*)wl_resource_get_user_data(res);
        delete resource;
    });
}

LResource::~LResource() noexcept
{
    m_client->imp()->resources.erase(m_clientLink);
    notifyDestruction();
}

void LResource::postErrorPrivate(UInt32 code, const std::string &message)
{
    if (client()->imp()->destroyed)
        return;

    client()->imp()->destroyed = true;
    LLog(CZWarning, CZLN, "Client protocol error: {}", message);
    wl_resource_post_error(resource(), code, "%s", message.c_str());
}

Int32 LResource::version() const noexcept
{
    return wl_resource_get_version(resource());
}

UInt32 LResource::id() const noexcept
{
    return wl_resource_get_id(resource());
}

void LResource::destroy()
{
    wl_resource_destroy(resource());
}
