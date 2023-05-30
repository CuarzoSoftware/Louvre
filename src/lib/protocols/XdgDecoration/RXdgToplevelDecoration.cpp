#include <protocols/XdgDecoration/private/RXdgToplevelDecorationPrivate.h>
#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>
#include <protocols/XdgDecoration/GXdgDecorationManager.h>

#include <private/LToplevelRolePrivate.h>

using namespace Louvre;
using namespace Louvre::Protocols::XdgDecoration;

static struct zxdg_toplevel_decoration_v1_interface xdg_toplevel_decoration_implementation
{
    .destroy = &RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::destroy,
    .set_mode = &RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::set_mode,
    .unset_mode = &RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::unset_mode
};

RXdgToplevelDecoration::RXdgToplevelDecoration
(
    GXdgDecorationManager *gXdgDecorationManager,
    LToplevelRole *lToplevelRole,
    UInt32 id
)
    :LResource
    (
        gXdgDecorationManager->client(),
        &zxdg_toplevel_decoration_v1_interface,
        gXdgDecorationManager->version(),
        id,
        &xdg_toplevel_decoration_implementation,
        &RXdgToplevelDecoration::RXdgToplevelDecorationPrivate::resource_destroy
    )
{
    m_imp = new RXdgToplevelDecorationPrivate();
    imp()->lToplevelRole = lToplevelRole;
    lToplevelRole->imp()->xdgDecoration = this;
    configure(lToplevelRole->imp()->pendingDecorationMode);
}

RXdgToplevelDecoration::~RXdgToplevelDecoration()
{
    if (lToplevelRole())
    {
        if (lToplevelRole()->decorationMode() == LToplevelRole::DecorationMode::ServerSide)
        {
            lToplevelRole()->imp()->pendingDecorationMode = LToplevelRole::DecorationMode::ClientSide;
            lToplevelRole()->imp()->decorationMode = LToplevelRole::DecorationMode::ClientSide;
            lToplevelRole()->decorationModeChanged();
        }

        lToplevelRole()->imp()->xdgDecoration = nullptr;
    }

    delete m_imp;
}

LToplevelRole *RXdgToplevelDecoration::lToplevelRole() const
{
    return imp()->lToplevelRole;
}

bool RXdgToplevelDecoration::configure(UInt32 mode) const
{
    zxdg_toplevel_decoration_v1_send_configure(resource(), mode);
    return true;
}
