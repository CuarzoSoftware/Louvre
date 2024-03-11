#include <protocols/Wayland/private/GOutputPrivate.h>
#include <LCompositor.h>

static struct wl_output_interface output_implementation =
{
#if LOUVRE_WL_OUTPUT_VERSION >= 3
    .release = &GOutput::GOutputPrivate::release
#endif
};

void GOutput::GOutputPrivate::bind(wl_client *client, void *output, UInt32 version, UInt32 id)
{
    LOutput *lOutput { static_cast<LOutput*>(output) };
    LClient *lClient { compositor()->getClientFromNativeResource(client) };

    new GOutput(lOutput,
                lClient,
                &wl_output_interface,
                version,
                id,
                &output_implementation);
}

#if LOUVRE_WL_OUTPUT_VERSION >= 3
void GOutput::GOutputPrivate::release(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}
#endif
