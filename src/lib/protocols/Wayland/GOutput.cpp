#include <protocols/Wayland/private/GOutputPrivate.h>

#include <private/LClientPrivate.h>

#include <LCompositor.h>
#include <LOutput.h>
#include <LOutputMode.h>

using namespace Protocols::Wayland;
using namespace std;

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
    )
{
    m_imp = new GOutputPrivate();
    imp()->output = output;

    this->client()->imp()->outputGlobals.push_back(this);
    imp()->clientLink = std::prev(this->client()->imp()->outputGlobals.end());

    sendConfiguration();
}

Louvre::Protocols::Wayland::GOutput::~GOutput()
{
    if (output())
        client()->imp()->outputGlobals.erase(imp()->clientLink);
    delete m_imp;
}

LOutput *GOutput::output() const
{
    return imp()->output;
}

void GOutput::sendConfiguration()
{
    wl_output_send_geometry(
        resource(),
        output()->rectC().x(),
        output()->rectC().y(),
        output()->physicalSize().w(),
        output()->physicalSize().h(),
        WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB,
        output()->manufacturer(),
        output()->model(),
        WL_OUTPUT_TRANSFORM_NORMAL);

    wl_output_send_mode(
        resource(),
        wl_output_mode::WL_OUTPUT_MODE_CURRENT,
        output()->currentMode()->sizeB().w(),
        output()->currentMode()->sizeB().h(),
        output()->currentMode()->refreshRate());

#if LOUVRE_OUTPUT_VERSION >= WL_OUTPUT_NAME_SINCE_VERSION
    if (version() >= WL_OUTPUT_NAME_SINCE_VERSION)
    {
        wl_output_send_name(resource(), output()->name());
        wl_output_send_description(resource(), output()->description());
    }
#endif

#if LOUVRE_OUTPUT_VERSION >= WL_OUTPUT_DONE_SINCE_VERSION
    if (version() >= WL_OUTPUT_DONE_SINCE_VERSION)
    {
        wl_output_send_scale(resource(), output()->scale());
        wl_output_send_done(resource());
    }
#endif
}
