#include <CZ/Louvre/Protocols/RelativePointer/relative-pointer-unstable-v1.h>
#include <CZ/Louvre/Protocols/RelativePointer/GRelativePointerManager.h>
#include <CZ/Louvre/Protocols/RelativePointer/RRelativePointer.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LUtils.h>

using namespace Louvre::Protocols::RelativePointer;

static const struct zwp_relative_pointer_manager_v1_interface imp
{
    .destroy = &GRelativePointerManager::destroy,
    .get_relative_pointer = &GRelativePointerManager::get_relative_pointer
};

void GRelativePointerManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GRelativePointerManager(client, version, id);
}

Int32 GRelativePointerManager::maxVersion() noexcept
{
    return LOUVRE_RELATIVE_POINTER_MANAGER_VERSION;
}

const wl_interface *GRelativePointerManager::interface() noexcept
{
    return &zwp_relative_pointer_manager_v1_interface;
}

GRelativePointerManager::GRelativePointerManager
    (wl_client *client,
        Int32 version,
        UInt32 id) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->relativePointerManagerGlobals.push_back(this);
}

GRelativePointerManager::~GRelativePointerManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->relativePointerManagerGlobals, this);
}

void GRelativePointerManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GRelativePointerManager::get_relative_pointer(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *pointer) noexcept
{
    new RRelativePointer(static_cast<Wayland::RPointer*>(wl_resource_get_user_data(pointer)),
                         id,
                         wl_resource_get_version(resource));
}
