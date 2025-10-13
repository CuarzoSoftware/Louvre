#include <CZ/Louvre/Protocols/XdgOutput/xdg-output-unstable-v1.h>
#include <CZ/Louvre/Protocols/XdgOutput/RXdgOutput.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/skia/core/SkPoint.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::XdgOutput;

static const struct zxdg_output_v1_interface imp
{
    .destroy = &RXdgOutput::destroy
};

RXdgOutput::RXdgOutput
    (
        Wayland::GOutput *outputRes,
        UInt32 id,
        Int32 version
        ) noexcept
    :LResource
    (
        outputRes->client(),
        &zxdg_output_v1_interface,
        version,
        id,
        &imp
        ),
    m_outputRes(outputRes)
{
    outputRes->m_xdgOutputRes.emplace_back(this);

    if (outputRes->output())
    {
        logicalPosition(outputRes->output()->pos());
        logicalSize(outputRes->output()->size());
        name(outputRes->output()->name().c_str());
        description(outputRes->output()->description().c_str());

        if (version >= 3 && outputRes->version() >= 2)
            outputRes->done();
        else
            done();
    }
}

RXdgOutput::~RXdgOutput() noexcept
{
    if (outputRes())
        CZVectorUtils::RemoveOneUnordered(outputRes()->m_xdgOutputRes, this);
}

/******************** REQUESTS ********************/


void RXdgOutput::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RXdgOutput::logicalPosition(SkIPoint pos) noexcept
{
    zxdg_output_v1_send_logical_position(resource(), pos.x(), pos.y());
}

void RXdgOutput::logicalSize(const SkISize &size) noexcept
{
    zxdg_output_v1_send_logical_size(resource(), size.width(), size.height());
}

void RXdgOutput::done() noexcept
{
    zxdg_output_v1_send_done(resource());
}

bool RXdgOutput::name(const char *name) noexcept
{
#if LOUVRE_XDG_OUTPUT_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        zxdg_output_v1_send_name(resource(), name);
        return true;
    }
#else
    L_UNUSED(name)
#endif
    return false;
}

bool RXdgOutput::description(const char *description) noexcept
{
#if LOUVRE_XDG_OUTPUT_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        zxdg_output_v1_send_description(resource(), description);
        return true;
    }
#else
    L_UNUSED(description)
#endif
    return false;
}
