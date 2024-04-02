#include <protocols/XdgOutput/xdg-output-unstable-v1.h>
#include <protocols/XdgOutput/RXdgOutput.h>
#include <protocols/Wayland/GOutput.h>
#include <LPoint.h>
#include <LUtils.h>

using namespace Louvre::Protocols::XdgOutput;

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
    outputRes->sendConfiguration();
}

RXdgOutput::~RXdgOutput() noexcept
{
    if (outputRes())
        LVectorRemoveOneUnordered(outputRes()->m_xdgOutputRes, this);
}

/******************** REQUESTS ********************/


void RXdgOutput::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RXdgOutput::logicalPosition(const LPoint &pos) noexcept
{
    zxdg_output_v1_send_logical_position(resource(), pos.x(), pos.y());
}

void RXdgOutput::logicalSize(const LSize &size) noexcept
{
    zxdg_output_v1_send_logical_size(resource(), size.x(), size.y());
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
