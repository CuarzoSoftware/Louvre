#include <CZ/Louvre/Protocols/WlrOutputManagement/wlr-output-management-unstable-v1.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputMode.h>
#include <LOutputMode.h>
#include <LUtils.h>
#include <CZ/skia/core/SkSize.h>

using namespace Louvre;
using namespace Louvre::Protocols::WlrOutputManagement;

static const struct zwlr_output_mode_v1_interface imp
{
    .release = RWlrOutputMode::release
};

RWlrOutputMode::RWlrOutputMode(
    RWlrOutputHead *wlrOutputHead,
    LOutputMode *mode
    ) noexcept
    :LResource
    (
        wlrOutputHead->client(),
        &zwlr_output_mode_v1_interface,
        wlrOutputHead->version(),
        0,
        &imp
    ),
    m_wlrOutputHead(wlrOutputHead),
    m_mode(mode)
{
    wlrOutputHead->m_modes.emplace_back(this);
    zwlr_output_head_v1_send_mode(wlrOutputHead->resource(), resource());
    size(mode->sizeB());
    refresh(mode->refreshRate());

    if (mode->isPreferred())
        preferred();
}

RWlrOutputMode::~RWlrOutputMode() noexcept
{
    if (m_wlrOutputHead)
        LVectorRemoveOneUnordered(m_wlrOutputHead->m_modes, this);
}

/******************** REQUESTS ********************/

void RWlrOutputMode::release(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

/******************** EVENTS ********************/

void RWlrOutputMode::size(const SkISize &size) noexcept
{
    if (m_wlrOutputHead)
        m_wlrOutputHead->markAsPendingDone();

    zwlr_output_mode_v1_send_size(resource(), size.width(), size.height());
}

void RWlrOutputMode::refresh(Int32 refresh) noexcept
{
    if (m_wlrOutputHead)
        m_wlrOutputHead->markAsPendingDone();

    zwlr_output_mode_v1_send_refresh(resource(), refresh);
}

void RWlrOutputMode::preferred() noexcept
{
    if (m_wlrOutputHead)
        m_wlrOutputHead->markAsPendingDone();

    zwlr_output_mode_v1_send_preferred(resource());
}

void RWlrOutputMode::finished() noexcept
{
    if (m_wlrOutputHead)
        m_wlrOutputHead->markAsPendingDone();

    zwlr_output_mode_v1_send_finished(resource());
}
