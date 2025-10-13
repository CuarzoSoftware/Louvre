#include <CZ/Louvre/Protocols/FractionalScale/fractional-scale-v1.h>
#include <CZ/Louvre/Protocols/FractionalScale/GFractionalScaleManager.h>
#include <CZ/Louvre/Protocols/FractionalScale/RFractionalScale.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::FractionalScale;

static const struct wp_fractional_scale_manager_v1_interface imp
{
    .destroy = &GFractionalScaleManager::destroy,
    .get_fractional_scale = &GFractionalScaleManager::get_fractional_scale
};

LGLOBAL_INTERFACE_IMP(GFractionalScaleManager, LOUVRE_FRACTIONAL_SCALE_MANAGER_VERSION, wp_fractional_scale_manager_v1_interface)

bool GFractionalScaleManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.FractionalScaleManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.FractionalScaleManager;
    return true;
}

GFractionalScaleManager::GFractionalScaleManager
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
    this->client()->imp()->fractionalScaleManagerGlobals.emplace_back(this);
}

GFractionalScaleManager::~GFractionalScaleManager() noexcept
{
    CZVectorUtils::RemoveOneUnordered(client()->imp()->fractionalScaleManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GFractionalScaleManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GFractionalScaleManager::get_fractional_scale(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto *surfaceRes { static_cast<Wayland::RWlSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->fractionalScaleRes())
    {
        surfaceRes->postError(
            WP_FRACTIONAL_SCALE_MANAGER_V1_ERROR_FRACTIONAL_SCALE_EXISTS,
            "The surface already has a fractional_scale object associated.");
        return;
    }

    new RFractionalScale(surfaceRes, id, wl_resource_get_version(resource));
}
