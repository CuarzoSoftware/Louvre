#include <protocols/BackgroundBlur/background-blur.h>
#include <protocols/BackgroundBlur/RBackgroundBlur.h>
#include <protocols/SvgPath/RSvgPath.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/RRegion.h>
#include <LSurface.h>

using namespace Louvre::Protocols::BackgroundBlur;

static const struct background_blur_interface imp
{
    .destroy = &RBackgroundBlur::destroy,
    .set_region = &RBackgroundBlur::set_region,
    .ack_configure = &RBackgroundBlur::ack_configure,
    .clear_clip = &RBackgroundBlur::clear_clip,
    .set_round_rect_clip = &RBackgroundBlur::set_round_rect_clip,
    .set_svg_path_clip = &RBackgroundBlur::set_svg_path_clip,
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
    blur.m_backgroundBlurRes.reset(this);
    blur.m_sentConfigurations.clear();

    if (blur.m_flags.check(LBackgroundBlur::Destroyed))
        surfaceRes->surface()->backgroundBlur()->reset();

    blur.configureRequest();

    if (!blur.m_flags.check(LBackgroundBlur::HasStateToSend))
        blur.configureState(blur.pendingConfiguration().state);

    if (!blur.m_flags.check(LBackgroundBlur::HasStyleToSend))
        blur.configureStyle(blur.pendingConfiguration().style);
}

RBackgroundBlur::~RBackgroundBlur() noexcept
{
    if (surfaceRes())
    {
        auto &blur { *surfaceRes()->surface()->backgroundBlur() };
        blur.m_flags.add(LBackgroundBlur::Destroyed);
    }
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
            blur.pendingProps().serial = blur.m_sentConfigurations.front().serial;
            blur.pendingProps().state = blur.m_sentConfigurations.front().state;
            blur.pendingProps().style = blur.m_sentConfigurations.front().style;
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

    if (region)
    {
        auto &rRegion { *static_cast<Protocols::Wayland::RRegion*>(wl_resource_get_user_data(region)) };
        blur.pendingProps().region = rRegion.region();
        blur.pendingProps().isEmpty = blur.pendingProps().region.empty();
        blur.pendingProps().isFullSize = false;
        blur.m_flags.add(LBackgroundBlur::RegionModified);
    }
    else if (!blur.pendingProps().isFullSize)
    {
        blur.pendingProps().region.clear();
        blur.pendingProps().isEmpty = false;
        blur.pendingProps().isFullSize = true;
        blur.m_flags.add(LBackgroundBlur::RegionModified);
    }
}

void RBackgroundBlur::clear_clip(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    if (blur.pendingProps().clipType == LBackgroundBlur::NoClip)
        return;

    blur.pendingProps().clipType = LBackgroundBlur::NoClip;
    blur.pendingProps().roundRectClip = LRRect();
    blur.pendingProps().svgPathClip.clear();
    blur.m_flags.add(LBackgroundBlur::ClipModified);
}

void RBackgroundBlur::set_round_rect_clip(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height, Int32 radTL, Int32 radTR, Int32 radBR, Int32 radBL)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    const LRRect newRRect { {x, y, width, height}, radTL, radTR, radBR, radBL };

    if (!newRRect.isValid())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_INVALID_ROUND_RECT, "invalid round rect size or radii");
        return;
    }

    if (blur.pendingProps().clipType == LBackgroundBlur::RoundRect && blur.pendingProps().roundRectClip == newRRect)
        return;

    blur.pendingProps().svgPathClip.clear();
    blur.pendingProps().roundRectClip = newRRect;
    blur.m_flags.add(LBackgroundBlur::ClipModified);
    blur.pendingProps().clipType = LBackgroundBlur::RoundRect;
}

void RBackgroundBlur::set_svg_path_clip(wl_client */*client*/, wl_resource *resource, wl_resource *svgPath)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };
    auto &rSvgPath { *static_cast<Protocols::SvgPath::RSvgPath*>(wl_resource_get_user_data(svgPath)) };

    if (!rSvgPath.isComplete())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_INVALID_SVG_PATH, "incomplete svg path");
        return;
    }

    blur.m_flags.add(LBackgroundBlur::ClipModified);
    blur.pendingProps().roundRectClip = LRRect();
    blur.pendingProps().svgPathClip = rSvgPath.commands();
    blur.pendingProps().clipType = LBackgroundBlur::SVGPath;
}
