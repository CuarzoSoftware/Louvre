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
    .set_round_rect = &RBackgroundBlur::set_round_rect,
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
    blur.m_flags.add(LBackgroundBlur::AssignedArea);
    blur.pendingProps().isFullSize = region == nullptr;
    blur.pendingProps().areaType = LBackgroundBlur::Region;
    blur.pendingProps().area.svgPath.clear();
    blur.pendingProps().area.roundRect = LRRect();

    if (region)
    {
        auto &rRegion { *static_cast<Protocols::Wayland::RRegion*>(wl_resource_get_user_data(region)) };
        blur.pendingProps().area.region = rRegion.region();

        const LBox &extents { blur.pendingProps().area.region.extents() };

        if (extents.x1 < 0 || extents.y1 < 0)
        {
            wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_OUT_OF_BOUNDS, "the region has negative extents");
            return;
        }

        blur.pendingProps().isEmpty = blur.pendingProps().area.region.empty();
    }
    else
    {
        blur.pendingProps().area.region.clear();
        blur.pendingProps().isEmpty = false;
    }
}

void RBackgroundBlur::set_round_rect(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height, Int32 radTL, Int32 radTR, Int32 radBR, Int32 radBL)
{
    if (x < 0 || y < 0)
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_OUT_OF_BOUNDS, "the round rect has a negative x or y value");
        return;
    }

    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };
    blur.m_flags.add(LBackgroundBlur::AssignedArea);
    blur.pendingProps().isFullSize = false;
    blur.pendingProps().area.region.clear();
    blur.pendingProps().area.svgPath.clear();

    blur.pendingProps().area.roundRect = LRRect({x, y, width, height}, radTL, radTR, radBR, radBL);

    if (!blur.pendingProps().area.roundRect.isValid())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_INVALID_ROUND_RECT, "invalid round rect size or radii");
        return;
    }

    if (width == 0 or height == 0)
    {
        blur.pendingProps().area.roundRect = LRRect();
        blur.pendingProps().areaType = LBackgroundBlur::Region;
        blur.pendingProps().isEmpty = true;
    }
    else if (radTL + radTR + radBR + radBL == 0)
    {
        blur.pendingProps().area.region.addRect(blur.pendingProps().area.roundRect);
        blur.pendingProps().area.roundRect = LRRect();
        blur.pendingProps().areaType = LBackgroundBlur::Region;
        blur.pendingProps().isEmpty = true;
    }
    else
    {
        blur.pendingProps().areaType = LBackgroundBlur::RoundRect;
        blur.pendingProps().isEmpty = false;
    }
}

void RBackgroundBlur::set_path(wl_client */*client*/, wl_resource *resource, wl_resource *svgPath)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };
    blur.m_flags.add(LBackgroundBlur::AssignedArea);
    blur.pendingProps().isFullSize = false;
    blur.pendingProps().area.region.clear();
    blur.pendingProps().area.roundRect = LRRect();

    auto &rSvgPath { *static_cast<Protocols::SvgPath::RSvgPath*>(wl_resource_get_user_data(svgPath)) };

    if (!rSvgPath.isComplete())
    {
        wl_resource_post_error(resource, BACKGROUND_BLUR_ERROR_INVALID_PATH, "invalid svg path");
        return;
    }

    blur.pendingProps().area.svgPath = rSvgPath.commands();

    if (blur.pendingProps().area.svgPath.empty())
    {
        blur.pendingProps().areaType = LBackgroundBlur::Region;
        blur.pendingProps().isEmpty = true;
    }
    else
    {
        blur.pendingProps().areaType = LBackgroundBlur::SVGPath;
        blur.pendingProps().isEmpty = false;
    }
}
