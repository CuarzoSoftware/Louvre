#include <globals/XdgDecoration/XdgToplevelDecoration.h>

#include <private/LToplevelRolePrivate.h>

void Louvre::Extensions::XdgDecoration::ToplevelDecoration::resource_destroy(wl_resource *resource)
{
    LToplevelRole *lToplevel = (LToplevelRole*)wl_resource_get_user_data(resource);

    if(lToplevel->decorationMode() == LToplevelRole::DecorationMode::ServerSide)
    {
        lToplevel->imp()->decorationMode = LToplevelRole::DecorationMode::ClientSide;
        lToplevel->decorationModeChanged();
    }

    lToplevel->imp()->xdgDecoration = nullptr;
}

void Louvre::Extensions::XdgDecoration::ToplevelDecoration::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void Louvre::Extensions::XdgDecoration::ToplevelDecoration::set_mode(wl_client *client, wl_resource *resource, UInt32 mode)
{
    L_UNUSED(client); L_UNUSED(resource); L_UNUSED(mode);
}

void Louvre::Extensions::XdgDecoration::ToplevelDecoration::unset_mode(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client); L_UNUSED(resource);
}
