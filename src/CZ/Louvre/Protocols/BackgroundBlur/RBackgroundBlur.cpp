#include <CZ/Louvre/Protocols/BackgroundBlur/lvr-background-blur.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/RBackgroundBlur.h>
#include <CZ/Louvre/Protocols/SvgPath/RSvgPath.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Protocols/Wayland/RRegion.h>
#include <LSurface.h>

using namespace Louvre::Protocols::BackgroundBlur;

static const struct lvr_background_blur_interface imp
{
    .destroy = &RBackgroundBlur::destroy,
    .set_region = &RBackgroundBlur::set_region,
    .ack_configure = &RBackgroundBlur::ack_configure,
    .clear_mask = &RBackgroundBlur::clear_mask,
    .set_round_rect_mask = &RBackgroundBlur::set_round_rect_mask,
    .set_svg_path_mask = &RBackgroundBlur::set_svg_path_mask,
};

RBackgroundBlur::RBackgroundBlur(
    CZBitset<LBackgroundBlur::MaskingCapabilities> maskCaps,
     Wayland::RSurface *surfaceRes,
     UInt32 id,
     Int32 version
     ) noexcept
    :LResource
    (
        surfaceRes->client(),
        &lvr_background_blur_interface,
        version,
        id,
        &imp
    ),
    m_surfaceRes(surfaceRes),
    m_maskingCapabilities(maskCaps)
{
    auto &blur { *surfaceRes->surface()->backgroundBlur() };
    blur.m_backgroundBlurRes.reset(this);
    blur.m_sentConfigurations.clear();

    if (blur.m_flags.has(LBackgroundBlur::Destroyed))
        surfaceRes->surface()->backgroundBlur()->reset();

    blur.configureRequest();

    if (!blur.m_flags.has(LBackgroundBlur::HasStateToSend))
        blur.configureState(blur.pendingConfiguration().state);

    if (!blur.m_flags.has(LBackgroundBlur::HasColorHintToSend))
        blur.configureColorHint(blur.pendingConfiguration().colorHint);
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
    lvr_background_blur_send_state(resource(), state);
}

void RBackgroundBlur::colorHint(LBackgroundBlur::ColorHint hint) noexcept
{
    lvr_background_blur_send_color_hint(resource(), hint);
}

void RBackgroundBlur::configure(UInt32 serial) noexcept
{
    lvr_background_blur_send_configure(resource(), serial);
}

/******************** REQUESTS ********************/

void RBackgroundBlur::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    wl_resource_destroy(resource);
}

void RBackgroundBlur::ack_configure(wl_client */*client*/, wl_resource *resource, UInt32 serial)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    while (!blur.m_sentConfigurations.empty())
    {
        if (blur.m_sentConfigurations.front().serial == serial)
        {
            blur.pendingProps().serial = blur.m_sentConfigurations.front().serial;
            blur.pendingProps().state = blur.m_sentConfigurations.front().state;
            blur.pendingProps().colorHint = blur.m_sentConfigurations.front().colorHint;
            blur.m_sentConfigurations.pop_front();
            return;
        }
        else
            blur.m_sentConfigurations.pop_front();
    }

    res.postError(LVR_BACKGROUND_BLUR_ERROR_INVALID_SERIAL, "invalid configure serial");
}

void RBackgroundBlur::set_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
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

void RBackgroundBlur::clear_mask(wl_client */*client*/, wl_resource *resource)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    // Noop
    if (res.m_maskingCapabilities.get() == 0)
        return;

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    if (blur.pendingProps().maskType == LBackgroundBlur::NoMask)
        return;

    blur.pendingProps().maskType = LBackgroundBlur::NoMask;
    blur.pendingProps().roundRectMask = LRRect();
    blur.pendingProps().svgPathMask.clear();
    blur.m_flags.add(LBackgroundBlur::MaskModified);
}

void RBackgroundBlur::set_round_rect_mask(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height, Int32 radTL, Int32 radTR, Int32 radBR, Int32 radBL)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    if (!res.m_maskingCapabilities.has(LBackgroundBlur::RoundRectMaskCap))
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_UNSUPPORTED_MASK, "the mask is not supported by the compositor");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };

    const LRRect newRRect { {x, y, width, height}, radTL, radTR, radBR, radBL };

    if (!newRRect.isValid())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_INVALID_ROUND_RECT, "invalid round rect size or radii");
        return;
    }

    if (blur.pendingProps().maskType == LBackgroundBlur::RoundRect && blur.pendingProps().roundRectMask == newRRect)
        return;

    blur.pendingProps().svgPathMask.clear();
    blur.pendingProps().roundRectMask = newRRect;
    blur.m_flags.add(LBackgroundBlur::MaskModified);
    blur.pendingProps().maskType = LBackgroundBlur::RoundRect;
}

void RBackgroundBlur::set_svg_path_mask(wl_client */*client*/, wl_resource *resource, wl_resource *svgPath)
{
    auto &res { *static_cast<RBackgroundBlur*>(wl_resource_get_user_data(resource)) };

    if (!res.surfaceRes())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_DESTROYED_SURFACE, "surface destroyed before object");
        return;
    }

    if (!res.m_maskingCapabilities.has(LBackgroundBlur::SVGPathMaskCap))
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_UNSUPPORTED_MASK, "the mask is not supported by the compositor");
        return;
    }

    auto &blur { *res.surfaceRes()->surface()->backgroundBlur() };
    auto &rSvgPath { *static_cast<Protocols::SvgPath::RSvgPath*>(wl_resource_get_user_data(svgPath)) };

    if (!rSvgPath.isComplete())
    {
        res.postError(LVR_BACKGROUND_BLUR_ERROR_INVALID_SVG_PATH, "incomplete svg path");
        return;
    }

    blur.m_flags.add(LBackgroundBlur::MaskModified);
    blur.pendingProps().roundRectMask = LRRect();
    blur.pendingProps().svgPathMask = rSvgPath.commands();
    blur.pendingProps().maskType = LBackgroundBlur::SVGPath;
}
