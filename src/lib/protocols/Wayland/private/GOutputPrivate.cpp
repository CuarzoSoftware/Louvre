#include <protocols/Wayland/private/GOutputPrivate.h>
#include <LOutput.h>
#include <LCompositor.h>
#include <LLog.h>

static struct wl_output_interface output_implementation =
{
#if LOUVRE_OUTPUT_VERSION >= WL_OUTPUT_RELEASE_SINCE_VERSION
    .release = &Louvre::Protocols::Wayland::GOutput::GOutputPrivate::release
#endif
};

void GOutput::GOutputPrivate::bind(wl_client *client, void *output, UInt32 version, UInt32 id)
{
    LOutput *lOutput = (LOutput*)output;

    LClient *lClient = lOutput->compositor()->getClientFromNativeResource(client);

    if (!lClient)
        return;

    for (GOutput *g : lClient->outputGlobals())
    {
        if (g->output() == lOutput)
        {
            LLog::warning("Client already bound to output %s. Ignoring it...", lOutput->name());
            return;
        }
    }

    new GOutput(lOutput,
                lClient,
                &wl_output_interface,
                version,
                id,
                &output_implementation,
                &GOutput::GOutputPrivate::resource_destroy);
}

void GOutput::GOutputPrivate::resource_destroy(wl_resource *resource)
{
    GOutput *gOutput = (GOutput*)wl_resource_get_user_data(resource);
    delete gOutput;
}

#if LOUVRE_OUTPUT_VERSION >= WL_OUTPUT_RELEASE_SINCE_VERSION
void GOutput::GOutputPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
