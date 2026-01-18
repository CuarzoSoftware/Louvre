#include <CZ/Louvre/Protocols/PrivateHandle/lvr-private-handle.h>
#include <CZ/Louvre/Protocols/PrivateHandle/GPrivateHandleManager.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PrivateHandle;

static const struct lvr_private_handle_manager_interface imp
{
    .destroy = &GPrivateHandleManager::destroy,
};

LGLOBAL_INTERFACE_IMP(GPrivateHandleManager, LOUVRE_PRIVATE_HANDLE_MANAGER_VERSION, lvr_private_handle_manager_interface)

bool GPrivateHandleManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.LvrPrivateHandleManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.LvrPrivateHandleManager;
    return true;
}

GPrivateHandleManager::GPrivateHandleManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
        )
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->privateHandleManagerGlobals.emplace_back(this);
    handle(this->client()->privateHandle());
}

GPrivateHandleManager::~GPrivateHandleManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->privateHandleManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GPrivateHandleManager::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void GPrivateHandleManager::handle(const std::string &handle) noexcept
{
    lvr_private_handle_manager_send_handle(resource(), handle.c_str());
}
