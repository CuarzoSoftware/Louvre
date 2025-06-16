#include <CZ/Louvre/Protocols/WlrOutputManagement/wlr-output-management-unstable-v1.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputConfigurationHead.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputConfiguration.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LUtils.h>
#include <CZ/skia/core/SkSize.h>
#include <LSeat.h>

using namespace Louvre;
using namespace Louvre::Protocols::WlrOutputManagement;

static const struct zwlr_output_configuration_v1_interface imp
{
    .enable_head = &RWlrOutputConfiguration::enable_head,
    .disable_head = &RWlrOutputConfiguration::disable_head,
    .apply = &RWlrOutputConfiguration::apply,
    .test = &RWlrOutputConfiguration::test,
    .destroy = &RWlrOutputConfiguration::destroy
};

RWlrOutputConfiguration::RWlrOutputConfiguration(
    GWlrOutputManager *wlrOutputManager,
    UInt32 id,
    UInt32 serial
    ) noexcept
    :LResource
    (
        wlrOutputManager->client(),
        &zwlr_output_configuration_v1_interface,
        wlrOutputManager->version(),
        id,
        &imp
    ),
    m_wlrOutputManager(wlrOutputManager),
    m_serial(serial)
{

}

RWlrOutputConfiguration::~RWlrOutputConfiguration() noexcept
{
    while (!m_enabled.empty())
        m_enabled.front()->destroy();
}

bool RWlrOutputConfiguration::checkAlreadyConfigured(LOutput *output) noexcept
{
    // If previously disabled
    for (std::size_t i = 0; i < m_disabled.size(); i++)
    {
        if (m_disabled[i].get() == output)
        {
            postError(ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_CONFIGURED_HEAD, "Head already configured.");
            return true;
        }
    }

    // If already enabled
    for (std::size_t i = 0; i < m_enabled.size(); i++)
    {
        if (m_enabled[i]->output() == output)
        {
            postError(ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_CONFIGURED_HEAD, "Head already configured.");
            return true;
        }
    }

    return false;
}

RWlrOutputConfiguration::Reply RWlrOutputConfiguration::validate() noexcept
{
    if (!m_wlrOutputManager || m_wlrOutputManager->lastDoneSerial() != m_serial)
        return Cancelled; // Configuration changed and the client doesn't know

    for (auto &wo : m_disabled)
        if (!wo.get())
            return Cancelled;

    for (auto *ch : m_enabled)
        if (!ch->output())
            return Cancelled;

    if (m_disabled.size() + m_enabled.size() != seat()->outputs().size())
        return Cancelled;

    return Succeeded;
}

/******************** REQUESTS ********************/

void RWlrOutputConfiguration::enable_head(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *head)
{
    auto &res { LRES_CAST(RWlrOutputConfiguration, resource) };

    if (res.m_replied)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED, "Configuration already used.");
        return;
    }

    auto &headRes { LRES_CAST(RWlrOutputHead, head) };

    if (headRes.output() && res.checkAlreadyConfigured(headRes.output()))
        return;

    // If headRes.output() is nullptr, the request will be cancelled later

    new RWlrOutputConfigurationHead(&res, id, headRes.output());
}

void RWlrOutputConfiguration::disable_head(wl_client */*client*/, wl_resource *resource, wl_resource *head)
{
    auto &res { LRES_CAST(RWlrOutputConfiguration, resource) };

    if (res.m_replied)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED, "Configuration already used.");
        return;
    }

    auto &headRes { LRES_CAST(RWlrOutputHead, head) };

    if (headRes.output())
    {
        if (res.checkAlreadyConfigured(headRes.output()))
            return;

        res.m_disabled.emplace_back(headRes.output());
    }

    // If headRes.output() is nullptr, the request will be cancelled later
}

void RWlrOutputConfiguration::apply(wl_client */*client*/, wl_resource *resource)
{
    auto &res { LRES_CAST(RWlrOutputConfiguration, resource) };

    if (res.m_replied)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED, "Configuration already used.");
        return;
    }

    Reply reply { res.validate() };

    if (reply == Succeeded)
    {
        std::vector<LSeat::OutputConfiguration> configurations, fallback;
        configurations.reserve(res.m_disabled.size() + res.m_enabled.size());
        fallback.reserve(seat()->outputs().size());

        // Save current conf
        for (LOutput *o : seat()->outputs())
            fallback.emplace_back(
                *o,
                o->state() != LOutput::Uninitialized,
                *o->currentMode(),
                o->pos(),
                o->transform(),
                o->fractionalScale());

        // Fill new configuration
        for (auto &wo : res.m_disabled)
            configurations.emplace_back(*wo.get(), false, *wo->currentMode(), wo->pos(), wo->transform(), wo->scale());

        for (auto *eh : res.m_enabled)
            configurations.emplace_back(
                *eh->output(),
                true,
                eh->m_mode.get() != nullptr ? *eh->m_mode.get() : *eh->output()->currentMode(),
                eh->m_setProps.has(RWlrOutputConfigurationHead::Position) ? eh->m_pos : eh->output()->pos(),
                eh->m_setProps.has(RWlrOutputConfigurationHead::Transform) ? eh->m_transform : eh->output()->transform(),
                eh->m_setProps.has(RWlrOutputConfigurationHead::Scale) ? eh->m_scale : eh->output()->fractionalScale());

        reply = seat()->configureOutputsRequest(res.client(), configurations) ? Succeeded : Failed;

        // Revert changes
        if (reply == Failed)
        {
            for (const auto &conf : fallback)
            {
                conf.output.setPos(conf.pos);
                conf.output.setTransform(conf.transform);
                conf.output.setScale(conf.scale);
                conf.output.setMode(&conf.mode);

                if (conf.initialized)
                    compositor()->addOutput(&conf.output);
                else
                    compositor()->removeOutput(&conf.output);
            }
        }
    }

    res.reply(reply);
}

void RWlrOutputConfiguration::test(wl_client */*client*/, wl_resource *resource)
{
    auto &res { LRES_CAST(RWlrOutputConfiguration, resource) };

    if (res.m_replied)
    {
        res.postError(ZWLR_OUTPUT_CONFIGURATION_V1_ERROR_ALREADY_USED, "Configuration already used.");
        return;
    }

    res.reply(res.validate());
}

void RWlrOutputConfiguration::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RWlrOutputConfiguration::reply(Reply reply) noexcept
{
    switch (reply)
    {
    case Failed:
        zwlr_output_configuration_v1_send_failed(resource());
        break;
    case Succeeded:
        zwlr_output_configuration_v1_send_succeeded(resource());
        break;
    case Cancelled:
        zwlr_output_configuration_v1_send_cancelled(resource());
        break;
    }

    m_replied = true;
}
