#include <protocols/GammaControl/RGammaControl.h>
#include <protocols/XdgOutput/RXdgOutput.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LClientPrivate.h>
#include <private/LOutputPrivate.h>
#include <LOutputMode.h>
#include <LUtils.h>
#include <LGlobal.h>

using namespace Protocols::Wayland;

static const struct wl_output_interface imp
{
#if LOUVRE_WL_OUTPUT_VERSION >= 3
    .release = &GOutput::release
#endif
};

void GOutput::bind(wl_client *client, void *output, UInt32 version, UInt32 id) noexcept
{
    LGlobal *outputGlobal { static_cast<LGlobal*>(output) };
    new GOutput((LOutput*)(outputGlobal->userData()), client, version, id);
}

Int32 GOutput::maxVersion() noexcept
{
    return LOUVRE_WL_OUTPUT_VERSION;
}

const wl_interface *GOutput::interface() noexcept
{
    return &wl_output_interface;
}

GOutput::GOutput(LOutput *output, wl_client *client, Int32 version, UInt32 id ) noexcept :
    LResource(client, interface(), version, id, &imp),
    m_output(output)
{
    this->client()->imp()->outputGlobals.emplace_back(this);
    sendConfiguration();
}

GOutput::~GOutput() noexcept
{
    if (output())
        LVectorRemoveOneUnordered(client()->imp()->outputGlobals, this);
}

/******************** REQUESTS ********************/

#if LOUVRE_WL_OUTPUT_VERSION >= 3
void GOutput::release(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
#endif

/******************** EVENTS ********************/

void GOutput::sendConfiguration() noexcept
{
    if (!output())
        return;

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

    for (auto *xdgOutput : xdgOutputRes())
    {
        xdgOutput->logicalPosition(output()->pos());
        xdgOutput->logicalSize(output()->size());
    }

    if (scale(output()->imp()->scale))
    {
        if (name(output()->name()))
            description(output()->description());

        for (auto *xdgOutput : xdgOutputRes())
            if (xdgOutput->version() <= 2)
                xdgOutput->done();

        done();
    }
    else
        for (auto *xdgOutput : xdgOutputRes())
            xdgOutput->done();
}

void GOutput::geometry(Int32 x, Int32 y, Int32 physicalWidth, Int32 physicalHeight,
                       Int32 subpixel, const char *make, const char *model, LTransform transform) noexcept
{
    wl_output_send_geometry(
        resource(),
        x, y,
        physicalWidth,
        physicalHeight,
        subpixel,
        make,
        model,
        static_cast<Int32>(transform));
}

void GOutput::mode(UInt32 flags, Int32 width, Int32 height, Int32 refresh) noexcept
{
    wl_output_send_mode(
        resource(),
        flags,
        width,
        height,
        refresh);
}

bool GOutput::done() noexcept
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

bool GOutput::scale(Int32 factor) noexcept
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

bool GOutput::name(const char *name) noexcept
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

bool GOutput::description(const char *description) noexcept
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
