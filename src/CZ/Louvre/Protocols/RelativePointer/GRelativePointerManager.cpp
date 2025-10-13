#include <CZ/Louvre/Protocols/RelativePointer/relative-pointer-unstable-v1.h>
#include <CZ/Louvre/Protocols/RelativePointer/GRelativePointerManager.h>
#include <CZ/Louvre/Protocols/RelativePointer/RRelativePointer.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::RelativePointer;

static const struct zwp_relative_pointer_manager_v1_interface imp
{
    .destroy = &GRelativePointerManager::destroy,
    .get_relative_pointer = &GRelativePointerManager::get_relative_pointer
};

LGLOBAL_INTERFACE_IMP(GRelativePointerManager, LOUVRE_RELATIVE_POINTER_MANAGER_VERSION, zwp_relative_pointer_manager_v1_interface)

bool GRelativePointerManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.RelativePointerManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.RelativePointerManager;
    return true;
}

GRelativePointerManager::GRelativePointerManager
    (wl_client *client,
        Int32 version,
        UInt32 id)
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->relativePointerManagerGlobals.push_back(this);
}

GRelativePointerManager::~GRelativePointerManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->relativePointerManagerGlobals, this);
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
