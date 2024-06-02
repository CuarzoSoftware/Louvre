#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>
#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <protocols/XdgDecoration/GXdgDecorationManager.h>
#include <private/LToplevelRolePrivate.h>

using namespace Louvre;
using namespace Louvre::Protocols::XdgDecoration;

static const struct zxdg_toplevel_decoration_v1_interface imp
{
    .destroy = &RXdgToplevelDecoration::destroy,
    .set_mode = &RXdgToplevelDecoration::set_mode,
    .unset_mode = &RXdgToplevelDecoration::unset_mode
};

RXdgToplevelDecoration::RXdgToplevelDecoration(
    GXdgDecorationManager *xdgDecorationManagerRes,
    LToplevelRole *toplevelRole,
    UInt32 id
) noexcept
    :LResource
    (
        xdgDecorationManagerRes->client(),
        &zxdg_toplevel_decoration_v1_interface,
        xdgDecorationManagerRes->version(),
        id,
        &imp
    ),
    m_toplevelRole(toplevelRole)
{
    toplevelRole->m_xdgDecorationRes.reset(this);
}

RXdgToplevelDecoration::~RXdgToplevelDecoration()
{
    if (toplevelRole() && toplevelRole()->decorationMode() == LToplevelRole::ServerSide)
    {
        toplevelRole()->m_preferredDecorationMode = LToplevelRole::ClientSide;
        toplevelRole()->configureDecorationMode(LToplevelRole::ClientSide);
    }
}

/******************** REQUESTS ********************/

void RXdgToplevelDecoration::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &xdgToplevelDecorationRes { *static_cast<RXdgToplevelDecoration*>(wl_resource_get_user_data(resource)) };

    if (!xdgToplevelDecorationRes.toplevelRole())
    {
        wl_resource_post_error(resource, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED, "Toplevel destroyed before decoration.");
        return;
    }

    wl_resource_destroy(resource);
}

void RXdgToplevelDecoration::set_mode(wl_client */*client*/, wl_resource *resource, UInt32 mode)
{
    auto &xdgToplevelDecorationRes { *static_cast<RXdgToplevelDecoration*>(wl_resource_get_user_data(resource)) };

    if (!xdgToplevelDecorationRes.toplevelRole())
    {
        wl_resource_post_error(resource, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED, "Toplevel destroyed before decoration.");
        return;
    }

    if (xdgToplevelDecorationRes.toplevelRole()->preferredDecorationMode() != mode)
    {
        xdgToplevelDecorationRes.toplevelRole()->m_preferredDecorationMode = (LToplevelRole::DecorationMode)mode;
        xdgToplevelDecorationRes.toplevelRole()->preferredDecorationModeChanged();
    }

    if (!xdgToplevelDecorationRes.toplevelRole()->m_flags.check(LToplevelRole::HasDecorationModeToSend))
        xdgToplevelDecorationRes.toplevelRole()->configureDecorationMode(xdgToplevelDecorationRes.toplevelRole()->pendingConfiguration().decorationMode);
}

void RXdgToplevelDecoration::unset_mode(wl_client */*client*/, wl_resource *resource)
{
    auto &xdgToplevelDecorationRes { *static_cast<RXdgToplevelDecoration*>(wl_resource_get_user_data(resource)) };

    if (!xdgToplevelDecorationRes.toplevelRole())
    {
        wl_resource_post_error(resource, ZXDG_TOPLEVEL_DECORATION_V1_ERROR_ORPHANED, "Toplevel destroyed before decoration.");
        return;
    }

    if (xdgToplevelDecorationRes.toplevelRole()->preferredDecorationMode() != 0)
    {
       xdgToplevelDecorationRes.toplevelRole()->m_preferredDecorationMode = LToplevelRole::NoPreferredMode;
       xdgToplevelDecorationRes.toplevelRole()->preferredDecorationModeChanged();
    }

    if (!xdgToplevelDecorationRes.toplevelRole()->m_flags.check(LToplevelRole::HasDecorationModeToSend))
        xdgToplevelDecorationRes.toplevelRole()->configureDecorationMode(xdgToplevelDecorationRes.toplevelRole()->pendingConfiguration().decorationMode);
}

/******************** EVENTS ********************/

void RXdgToplevelDecoration::configure(UInt32 mode) noexcept
{
    zxdg_toplevel_decoration_v1_send_configure(resource(), mode);
}
