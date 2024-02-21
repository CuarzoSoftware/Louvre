#include <protocols/XdgDecoration/private/RXdgToplevelDecorationPrivate.h>
#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>
#include <private/LToplevelRolePrivate.h>

void RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    RXdgToplevelDecoration *rXdgToplevelDecoration = (RXdgToplevelDecoration*)wl_resource_get_user_data(resource);

    if (!rXdgToplevelDecoration->toplevelRole())
    {
        wl_resource_post_error(resource, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED, "Toplevel destroyed before decoration.");
        return;
    }

    wl_resource_destroy(resource);
}

void RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::set_mode(wl_client *client, wl_resource *resource, UInt32 mode)
{
    L_UNUSED(client);

    RXdgToplevelDecoration *rXdgToplevelDecoration = (RXdgToplevelDecoration*)wl_resource_get_user_data(resource);

    if (!rXdgToplevelDecoration->toplevelRole())
        return;

    if (rXdgToplevelDecoration->toplevelRole()->preferredDecorationMode() != mode)
    {
        rXdgToplevelDecoration->toplevelRole()->imp()->preferredDecorationMode = (LToplevelRole::DecorationMode)mode;
        rXdgToplevelDecoration->toplevelRole()->preferredDecorationModeChanged();
    }
}

void RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::unset_mode(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    RXdgToplevelDecoration *rXdgToplevelDecoration = (RXdgToplevelDecoration*)wl_resource_get_user_data(resource);

    if (!rXdgToplevelDecoration->toplevelRole())
        return;

    if (rXdgToplevelDecoration->toplevelRole()->preferredDecorationMode() != 0)
    {
        rXdgToplevelDecoration->toplevelRole()->imp()->preferredDecorationMode = LToplevelRole::NoPreferredMode;
        rXdgToplevelDecoration->toplevelRole()->preferredDecorationModeChanged();
    }
}
