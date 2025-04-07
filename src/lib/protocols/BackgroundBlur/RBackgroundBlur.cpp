#include <protocols/BackgroundBlur/background-blur.h>
#include <protocols/BackgroundBlur/RBackgroundBlur.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/RRegion.h>
#include <LSurface.h>

using namespace Louvre::Protocols::BackgroundBlur;

static const struct background_blur_interface imp
{
    .destroy = &RBackgroundBlur::destroy,
    .set_region = &RBackgroundBlur::set_region,
    .ack_configure = &RBackgroundBlur::ack_configure,
    .set_path = &RBackgroundBlur::set_path,
};

RBackgroundBlur::RBackgroundBlur
    (
        Wayland::RSurface *surfaceRes,
        UInt32 id,
        Int32 version
        ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &background_blur_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes)
{
    auto &blur { *surfaceRes->surface()->backgroundBlur() };
    blur.m_flags.remove(LBackgroundBlur::Destroyed);
    blur.m_backgroundBlurRes.reset(this);
    blur.configureRequest();

    if (!blur.m_flags.check(LBackgroundBlur::HasStateToSend))
        blur.configureState(blur.pendingConfiguration().state);

    if (!blur.m_flags.check(LBackgroundBlur::HasStyleToSend))
        blur.configureStyle(blur.pendingConfiguration().style);
}

RBackgroundBlur::~RBackgroundBlur() noexcept
{
    if (surfaceRes())
        surfaceRes()->surface()->backgroundBlur()->m_flags.add(LBackgroundBlur::Destroyed);
}

/******************** EVENTS ********************/

void RBackgroundBlur::state(LBackgroundBlur::State state) noexcept
{
    background_blur_send_state(resource(), state);
}

void RBackgroundBlur::style(LBackgroundBlur::Style style) noexcept
{
    background_blur_send_style(resource(), style);
}

void RBackgroundBlur::configure(UInt32 serial) noexcept
{
    background_blur_send_configure(resource(), serial);
}

/******************** REQUESTS ********************/

void RBackgroundBlur::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    wl_resource_destroy(resource);
}

void RBackgroundBlur::ack_configure(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    while (!blur.m_sentConfigurations.empty())
    {
        if (blur.m_sentConfigurations.front().serial == serial)
        {
            blur.pendingAtoms().serial = blur.m_sentConfigurations.front().serial;
            blur.pendingAtoms().state = blur.m_sentConfigurations.front().state;
            blur.pendingAtoms().style = blur.m_sentConfigurations.front().style;
            blur.m_sentConfigurations.pop_front();
            return;
        }
        else
            blur.m_sentConfigurations.pop_front();
    }

    wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_INVALID_SERIAL, "invalid configure serial");
}

void RBackgroundBlur::set_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };
    blur.m_flags.add(LBackgroundBlur::AssignedRegion);

    if (region)
    {
        auto &rRegion { *static_cast<Protocols::Wayland::RRegion*>(wl_resource_get_user_data(region)) };
        blur.pendingAtoms().region.reset(new LRegion(rRegion.region()));
        blur.pendingAtoms().path.reset();
    }
    else
    {
        blur.pendingAtoms().region.reset();
        blur.pendingAtoms().path.reset();
    }
}

void RBackgroundBlur::set_path(wl_client */*client*/, wl_resource *resource, const char *svgPath)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };
    blur.m_flags.add(LBackgroundBlur::AssignedRegion);

    if (svgPath)
    {
        blur.pendingAtoms().path.reset(new std::string(svgPath));
        blur.pendingAtoms().region.reset();
    }
    else
    {
        blur.pendingAtoms().region.reset();
        blur.pendingAtoms().path.reset();
    }
}
