#include <CZ/Louvre/Protocols/Wayland/GDataDeviceManager.h>
#include <CZ/Louvre/Protocols/Wayland/RDataDevice.h>
#include <CZ/Louvre/Protocols/Wayland/RDataSource.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Core/Utils/CZVectorUtils.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ::Protocols::Wayland;

static const struct wl_data_device_manager_interface imp
{
    .create_data_source = &GDataDeviceManager::create_data_source,
    .get_data_device = &GDataDeviceManager::get_data_device
};

LGLOBAL_INTERFACE_IMP(GDataDeviceManager, LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION, wl_data_device_manager_interface)

bool GDataDeviceManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.WlDataDeviceManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.WlDataDeviceManager;
    return true;
}

GDataDeviceManager::GDataDeviceManager
    (
        wl_client *client,
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
    this->client()->imp()->dataDeviceManagerGlobals.push_back(this);
}

GDataDeviceManager::~GDataDeviceManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->dataDeviceManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GDataDeviceManager::create_data_source(wl_client */*client*/, wl_resource *resource, UInt32 id) noexcept
{
    new RDataSource(static_cast<GDataDeviceManager*>(wl_resource_get_user_data(resource)), id);
}

void GDataDeviceManager::get_data_device(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *seat) noexcept
{
    GSeat *seatRes { static_cast<GSeat*>(wl_resource_get_user_data(seat)) };
    auto *res { static_cast<GDataDeviceManager*>(wl_resource_get_user_data(resource)) };

    if (seatRes->dataDeviceRes())
    {
        res->postError(0, "The wl_seat already has a wl_data_device");
        return;
    }

    new RDataDevice(res, seatRes, id);
}
