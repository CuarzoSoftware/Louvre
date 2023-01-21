#include <globals/Wayland/Output.h>

#include <private/LClientPrivate.h>

#include <LCompositor.h>
#include <LOutput.h>
#include <LOutputMode.h>

using namespace std;

static struct wl_output_interface output_implementation = {&Louvre::Globals::Output::release};

#if LOUVRE_OUTPUT_VERSION >= 3
void Louvre::Globals::Output::release(wl_client *,wl_resource *resource)
{
    wl_resource_destroy(resource);
}
#endif

void Louvre::Globals::Output::resource_destroy(wl_resource *resource)
{
    LOutput *lOutput = (LOutput*)wl_resource_get_user_data(resource);

    wl_client *client = wl_resource_get_client(resource);

    for(LClient *c : lOutput->compositor()->clients())
    {
        if(c->client() == client)
        {
            c->imp()->outputResources.remove(resource);
            return;
        }
    }
}

void Louvre::Globals::Output::bind(wl_client *client, void *data, UInt32 version, UInt32 id)
{
    LOutput* output = (LOutput*)data;
    LClient *lClient = nullptr;

    for(LClient *c : output->compositor()->clients())
    {
        if(c->client() == client)
        {
            lClient = c;
            break;
        }
    }

    if(!lClient)
        return;

    wl_resource *resource = wl_resource_create(client, &wl_output_interface, version, id);
    wl_resource_set_implementation(resource, &output_implementation, output, &Output::resource_destroy);
    sendConfiguration(resource, output);
    lClient->imp()->outputResources.push_back(resource);
}

void Globals::Output::sendConfiguration(wl_resource *resource, LOutput *output)
{
    Int32 version = wl_resource_get_version(resource);

    wl_output_send_geometry(
        resource,
        output->rectC().x(),
        output->rectC().y(),
        output->physicalSize().w(),
        output->physicalSize().h(),
        WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB,
        output->manufacturer(),
        output->model(),
        WL_OUTPUT_TRANSFORM_NORMAL);

    wl_output_send_mode(
        resource,
        wl_output_mode::WL_OUTPUT_MODE_CURRENT,
        output->currentMode()->sizeB().w(),
        output->currentMode()->sizeB().h(),
        output->currentMode()->refreshRate());

#if LOUVRE_OUTPUT_VERSION >= 4
    if(version >= 4)
    {
        wl_output_send_name(resource, output->name());
        wl_output_send_description(resource, output->description());
    }
#endif

#if LOUVRE_OUTPUT_VERSION >= 2
    if(version >= 2)
    {
        wl_output_send_scale(resource, output->scale());
        wl_output_send_done(resource);
    }
#endif
}
