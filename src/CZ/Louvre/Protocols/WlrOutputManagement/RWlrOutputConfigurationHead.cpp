#include <CZ/Louvre/Protocols/WlrOutputManagement/wlr-output-management-unstable-v1.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputConfigurationHead.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputConfiguration.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputMode.h>
#include <LOutputMode.h>
#include <LOutput.h>
#include <LUtils.h>
#include <CZ/skia/core/SkSize.h>

using namespace Louvre;
using namespace Louvre::Protocols::WlrOutputManagement;

static const struct zwlr_output_configuration_head_v1_interface imp
{
    .set_mode = &RWlrOutputConfigurationHead::set_mode,
    .set_custom_mode = &RWlrOutputConfigurationHead::set_custom_mode,
    .set_position = &RWlrOutputConfigurationHead::set_position,
    .set_transform = &RWlrOutputConfigurationHead::set_transform,
    .set_scale = &RWlrOutputConfigurationHead::set_scale,
#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 4
    .set_adaptive_sync = &RWlrOutputConfigurationHead::set_adaptive_sync,
#endif
};

RWlrOutputConfigurationHead::RWlrOutputConfigurationHead(
        RWlrOutputConfiguration *wlrOutputConfiguration,
        UInt32 id,
        LOutput *output
    ) noexcept
    :LResource
    (
        wlrOutputConfiguration->client(),
        &zwlr_output_configuration_head_v1_interface,
        wlrOutputConfiguration->version(),
        id,
        &imp
    ),
    m_wlrOutputConfiguration(wlrOutputConfiguration),
    m_output(output)
{
    wlrOutputConfiguration->m_enabled.emplace_back(this);
}

RWlrOutputConfigurationHead::~RWlrOutputConfigurationHead() noexcept
{
    if (m_wlrOutputConfiguration)
        LVectorRemoveOneUnordered(m_wlrOutputConfiguration->m_enabled, this);
}

/******************** REQUESTS ********************/

void RWlrOutputConfigurationHead::set_mode(wl_client */*client*/, wl_resource *resource, wl_resource *mode)
{
    auto &res { LRES_CAST(RWlrOutputConfigurationHead, resource) };

    if (res.m_setProps.has(Mode))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Mode already set.");
        return;
    }

    res.m_setProps.add(Mode);

    if (!res.m_output) return; // Maybe unplugged, will be cancelled later...

    if (res.m_setProps.has(CustomMode))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Can't set both mode and custom mode.");
        return;
    }

    auto &modeRes { LRES_CAST(RWlrOutputMode, mode) };

    if (!modeRes.mode() || modeRes.mode()->output() != res.m_output)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_MODE, "Invalid mode.");
        return;
    }

    res.m_mode.reset(modeRes.mode());
}

void RWlrOutputConfigurationHead::set_custom_mode(wl_client */*client*/, wl_resource *resource, Int32 width, Int32 height, Int32 refresh)
{
    auto &res { LRES_CAST(RWlrOutputConfigurationHead, resource) };

    if (res.m_setProps.has(CustomMode))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Custom mode already set.");
        return;
    }

    res.m_setProps.add(CustomMode);

    if (!res.m_output) return; // Maybe unplugged, will be cancelled later...

    if (res.m_setProps.has(Mode))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Can't set both mode and custom mode.");
        return;
    }

    /* Custom modes are not supported, look for the closest one */

    LOutputMode *bestMode { nullptr };

    Int64 bestScore { std::numeric_limits<Int64>::max() };

    for (LOutputMode *m : res.m_output->modes())
    {
        Int64 score = (std::abs(m->sizeB().width() - width) + std::abs(m->sizeB().height() - height)) * 100000;

        if (refresh <= 0)
            score -= m->refreshRate();
        else
            score += std::abs((Int32)m->refreshRate() - refresh);

        if (score < bestScore)
        {
            bestMode = m;
            bestScore = score;
        }
    }

    if (!bestMode)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_CUSTOM_MODE, "Invalid custom mode.");
        return;
    }

    res.m_mode.reset(bestMode);
}

void RWlrOutputConfigurationHead::set_position(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y)
{
    auto &res { LRES_CAST(RWlrOutputConfigurationHead, resource) };

    if (res.m_setProps.has(Position))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Position already set.");
        return;
    }

    res.m_setProps.add(Position);

    if (!res.m_output) return; // Maybe unplugged, will be cancelled later...

    res.m_pos.fX = x;
    res.m_pos.fY = y;
}

void RWlrOutputConfigurationHead::set_scale(wl_client */*client*/, wl_resource *resource, Float24 scale)
{
    auto &res { LRES_CAST(RWlrOutputConfigurationHead, resource) };

    if (res.m_setProps.has(Scale))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Scale already set.");
        return;
    }

    res.m_setProps.add(Scale);

    if (!res.m_output) return; // Maybe unplugged, will be cancelled later...

    res.m_scale = wl_fixed_to_double(scale);

    if (res.m_scale <= 0)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_SCALE, "Invalid scale <= 0.");
        return;
    }
}

void RWlrOutputConfigurationHead::set_transform(wl_client */*client*/, wl_resource *resource, Int32 transform)
{
    auto &res { LRES_CAST(RWlrOutputConfigurationHead, resource) };

    if (res.m_setProps.has(Transform))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Transform already set.");
        return;
    }

    res.m_setProps.add(Transform);

    if (!res.m_output) return; // Maybe unplugged, will be cancelled later...

    if (transform < 0 || transform > 7)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_INVALID_TRANSFORM, "Invalid transform.");
        return;
    }

    res.m_transform = (CZTransform)transform;
}

#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 4
void RWlrOutputConfigurationHead::set_adaptive_sync(wl_client */*client*/, wl_resource *resource, UInt32 vrr)
{
    auto &res { LRES_CAST(RWlrOutputConfigurationHead, resource) };

    if (res.m_setProps.has(VRR))
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_HEAD_V1_ERROR_ALREADY_SET, "Adaptive sync mode already set.");
        return;
    }

    res.m_setProps.add(VRR);

    if (!res.m_output) return; // Maybe unplugged, will be cancelled later...

    res.m_vrr = vrr;
}
#endif
