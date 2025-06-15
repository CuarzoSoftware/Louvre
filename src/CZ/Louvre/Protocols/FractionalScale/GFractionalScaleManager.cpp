#include <CZ/Louvre/Protocols/FractionalScale/fractional-scale-v1.h>
#include <CZ/Louvre/Protocols/FractionalScale/GFractionalScaleManager.h>
#include <CZ/Louvre/Protocols/FractionalScale/RFractionalScale.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <LUtils.h>

using namespace Louvre::Protocols::FractionalScale;

static const struct wp_fractional_scale_manager_v1_interface imp
{
    .destroy = &GFractionalScaleManager::destroy,
    .get_fractional_scale = &GFractionalScaleManager::get_fractional_scale
};

void GFractionalScaleManager::bind(wl_client *client, void */*data*/, UInt32 version, UInt32 id) noexcept
{
    new GFractionalScaleManager(client, version, id);
}

Int32 GFractionalScaleManager::maxVersion() noexcept
{
    return LOUVRE_FRACTIONAL_SCALE_MANAGER_VERSION;
}

const wl_interface *GFractionalScaleManager::interface() noexcept
{
    return &wp_fractional_scale_manager_v1_interface;
}

GFractionalScaleManager::GFractionalScaleManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        interface(),
        version,
        id,
        &imp
    )
{
    this->client()->imp()->fractionalScaleManagerGlobals.emplace_back(this);
}

GFractionalScaleManager::~GFractionalScaleManager() noexcept
{
    LVectorRemoveOneUnordered(client()->imp()->fractionalScaleManagerGlobals, this);
}

/******************** REQUESTS ********************/

void GFractionalScaleManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

void GFractionalScaleManager::get_fractional_scale(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *surface) noexcept
{
    auto *surfaceRes { static_cast<Wayland::RSurface*>(wl_resource_get_user_data(surface)) };

    if (surfaceRes->fractionalScaleRes())
    {
        surfaceRes->postError(
            WP_FRACTIONAL_SCALE_MANAGER_V1_ERROR_FRACTIONAL_SCALE_EXISTS,
            "The surface already has a fractional_scale object associated.");
        return;
    }

    new RFractionalScale(surfaceRes, id, wl_resource_get_version(resource));
}
