#include <protocols/RelativePointer/private/GRelativePointerManagerPrivate.h>
#include <protocols/RelativePointer/relative-pointer-unstable-v1.h>
#include <protocols/RelativePointer/RRelativePointer.h>
#include <protocols/Wayland/RPointer.h>

struct zwp_relative_pointer_manager_v1_interface zwp_relative_pointer_manager_v1_implementation =
{
    .destroy = &GRelativePointerManager::GRelativePointerManagerPrivate::destroy,
    .get_relative_pointer = &GRelativePointerManager::GRelativePointerManagerPrivate::get_relative_pointer
};

void GRelativePointerManager::GRelativePointerManagerPrivate::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    L_UNUSED(data)
    new GRelativePointerManager(client,
                         &zwp_relative_pointer_manager_v1_interface,
                         version,
                         id,
                         &zwp_relative_pointer_manager_v1_implementation);
}

void GRelativePointerManager::GRelativePointerManagerPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client)
    wl_resource_destroy(resource);
}

void GRelativePointerManager::GRelativePointerManagerPrivate::get_relative_pointer(wl_client *client, wl_resource *resource, UInt32 id, wl_resource *pointer)
{
    L_UNUSED(client)
    Wayland::RPointer *rPointer { (Wayland::RPointer*)wl_resource_get_user_data(pointer) };
    new RRelativePointer(rPointer, id, wl_resource_get_version(resource));
}
