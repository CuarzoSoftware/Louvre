#include <protocols/Wayland/private/GOutputPrivate.h>
#include <protocols/GammaControl/private/RGammaControlPrivate.h>
#include <private/LClientPrivate.h>
#include <LCompositor.h>
#include <LOutputMode.h>
#include <private/LOutputPrivate.h>

using namespace Protocols::Wayland;

GOutput::GOutput
(
    LOutput *output,
    LClient *lClient,
    const wl_interface *interface,
    Int32 version,
    UInt32 id,
    const void *implementation,
    wl_resource_destroy_func_t destroy
)
    :LResource
    (
        lClient,
        interface,
        version,
        id,
        implementation,
        destroy
    ),
    LPRIVATE_INIT_UNIQUE(GOutput)
{
    imp()->lOutput = output;
    this->client()->imp()->outputGlobals.push_back(this);
    sendConfiguration();
}

GOutput::~GOutput()
{
    if (output())
        LVectorRemoveOneUnordered(client()->imp()->outputGlobals, this);

    for (GammaControl::RGammaControl *rGammaControl : imp()->gammaControlResources)
        rGammaControl->imp()->gOutput = nullptr;
}

LOutput *GOutput::output() const
{
    return imp()->lOutput;
}

void GOutput::sendConfiguration()
{
    geometry(
        output()->pos().x(),
        output()->pos().y(),
        output()->physicalSize().w(),
        output()->physicalSize().h(),
        output()->subPixel(),
        output()->manufacturer(),
        output()->model(),
        output()->transform());

    mode(
        WL_OUTPUT_MODE_CURRENT,
        output()->currentMode()->sizeB().w(),
        output()->currentMode()->sizeB().h(),
        output()->currentMode()->refreshRate());

    if (scale(output()->imp()->scale))
    {
        if (name(output()->name()))
            description(output()->description());
        done();
    }
}

bool GOutput::geometry(Int32 x, Int32 y, Int32 physicalWidth, Int32 physicalHeight,
                       Int32 subpixel, const char *make, const char *model, Int32 transform)
{
    wl_output_send_geometry(
        resource(),
        x, y,
        physicalWidth,
        physicalHeight,
        subpixel,
        make,
        model,
        transform);
    return true;
}

bool GOutput::mode(UInt32 flags, Int32 width, Int32 height, Int32 refresh)
{
    wl_output_send_mode(
        resource(),
        flags,
        width,
        height,
        refresh);
    return true;
}

bool GOutput::done()
{
#if LOUVRE_WL_OUTPUT_VERSION >= 2
    if (version() >= 2)
    {
        wl_output_send_done(resource());
        return true;
    }
#endif
    return false;
}

bool GOutput::scale(Int32 factor)
{
#if LOUVRE_WL_OUTPUT_VERSION >= 2
    if (version() >= 2)
    {
        wl_output_send_scale(resource(), factor);
        return true;
    }
#endif
    L_UNUSED(factor);
    return false;
}

bool GOutput::name(const char *name)
{
#if LOUVRE_WL_OUTPUT_VERSION >= 4
    if (version() >= 4)
    {
        wl_output_send_name(resource(), name);
        return true;
    }
#endif
    L_UNUSED(name);
    return false;
}

bool GOutput::description(const char *description)
{
#if LOUVRE_WL_OUTPUT_VERSION >= 4
    if (version() >= 4)
    {
        wl_output_send_description(resource(), description);
        return true;
    }
#endif
    L_UNUSED(description);
    return false;
}
