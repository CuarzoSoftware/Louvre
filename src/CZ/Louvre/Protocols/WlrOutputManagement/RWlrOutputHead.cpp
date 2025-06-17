#include <CZ/Louvre/Protocols/WlrOutputManagement/wlr-output-management-unstable-v1.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputHead.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/RWlrOutputMode.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/LUtils.h>
#include <CZ/skia/core/SkSize.h>

using namespace Louvre;
using namespace Louvre::Protocols::WlrOutputManagement;

static const struct zwlr_output_head_v1_interface imp
{
#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 3
    .release = RWlrOutputHead::release
#endif
};

RWlrOutputHead::RWlrOutputHead(
    GWlrOutputManager *wlrOutputManager,
    LOutput *output
    ) noexcept
    :LResource
    (
        wlrOutputManager->client(),
        &zwlr_output_head_v1_interface,
        wlrOutputManager->version(),
        0,
        &imp
    ),
    m_wlrOutputManager(wlrOutputManager),
    m_output(output)
{
    wlrOutputManager->m_heads.emplace_back(this);
    output->imp()->wlrOutputHeads.emplace_back(this);
    zwlr_output_manager_v1_send_head(wlrOutputManager->resource(), resource());
    name(output->name());
    description(output->description());
    physicalSize(output->physicalSize());

    RWlrOutputMode *currMode { nullptr };
    for (LOutputMode *m : output->modes())
    {
        if (m == output->currentMode())
            currMode = mode(m);
        else
            mode(m);
    }

    enabled(output->state() != LOutput::Uninitialized);

    if (currMode)
        currentMode(currMode);

    position(output->pos());
    transform(output->transform());
    scale(output->fractionalScale());

    if (make(output->manufacturer()))
    {
        model(output->model());

        if (output->serialNumber())
            serialNumber(output->serialNumber());

        adaptiveSync(false);
    }
}

RWlrOutputHead::~RWlrOutputHead() noexcept
{
    if (m_wlrOutputManager)
        LVectorRemoveOneUnordered(m_wlrOutputManager->m_heads, this);

    if (m_output)
        LVectorRemoveOneUnordered(m_output->imp()->wlrOutputHeads, this);
}

void RWlrOutputHead::markAsPendingDone() noexcept
{
    if (m_wlrOutputManager)
        m_wlrOutputManager->m_pendingDone = true;
}

/******************** REQUESTS ********************/

#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 3
void RWlrOutputHead::release(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}
#endif

/******************** EVENTS ********************/

void RWlrOutputHead::name(const char *name) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_name(resource(), name);
}

void RWlrOutputHead::description(const char *description) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_description(resource(), description);
}

void RWlrOutputHead::physicalSize(const SkISize &size) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_physical_size(resource(), size.width(), size.height());
}

RWlrOutputMode *RWlrOutputHead::mode(LOutputMode *mode) noexcept
{
    for (auto *m : m_modes)
        if (m->mode() == mode)
            return nullptr;

    return new RWlrOutputMode(this, mode);
}

void RWlrOutputHead::enabled(bool enabled) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_enabled(resource(), enabled);
}

void RWlrOutputHead::currentMode(RWlrOutputMode *mode) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_current_mode(resource(), mode->resource());
}

void RWlrOutputHead::position(SkIPoint pos) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_position(resource(), pos.x(), pos.y());
}

void RWlrOutputHead::transform(CZTransform transform) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_transform(resource(), (Int32)transform);
}

void RWlrOutputHead::scale(Float32 scale) noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_scale(resource(), wl_fixed_from_double(scale));
}

void RWlrOutputHead::finished() noexcept
{
    markAsPendingDone();
    zwlr_output_head_v1_send_finished(resource());
}

bool RWlrOutputHead::make(const char *make) noexcept
{
#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        markAsPendingDone();
        zwlr_output_head_v1_send_make(resource(), make);
        return true;
    }
#else
    L_UNUSED(make)
#endif
    return false;
}

bool RWlrOutputHead::model(const char *model) noexcept
{
#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        markAsPendingDone();
        zwlr_output_head_v1_send_model(resource(), model);
        return true;
    }
#else
    L_UNUSED(model)
#endif
    return false;
}

bool RWlrOutputHead::serialNumber(const char *serial) noexcept
{
#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 2
    if (version() >= 2)
    {
        markAsPendingDone();
        zwlr_output_head_v1_send_serial_number(resource(), serial);
        return true;
    }
#else
    L_UNUSED(serial)
#endif
    return false;
}

bool RWlrOutputHead::adaptiveSync(bool enabled) noexcept
{
#if LOUVRE_WLR_OUTPUT_MANAGER_VERSION >= 4
    if (version() >= 4)
    {
        markAsPendingDone();
        zwlr_output_head_v1_send_adaptive_sync(resource(), enabled);
        return true;
    }
#else
    L_UNUSED(enabled)
#endif
    return false;
}
