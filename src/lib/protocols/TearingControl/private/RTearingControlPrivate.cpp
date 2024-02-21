#include <protocols/TearingControl/private/RTearingControlPrivate.h>
#include <protocols/TearingControl/tearing-control-v1.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LSurfacePrivate.h>

void RTearingControl::RTearingControlPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RTearingControl::RTearingControlPrivate::set_presentation_hint(wl_client *client, wl_resource *resource, UInt32 hint)
{
    L_UNUSED(client)

    RTearingControl *rTearingControl = (RTearingControl*)wl_resource_get_user_data(resource);
    rTearingControl->imp()->preferVSync = hint == WP_TEARING_CONTROL_V1_PRESENTATION_HINT_VSYNC;
}
