#include <protocols/XdgShell/private/RXdgPositionerPrivate.h>

#include <protocols/XdgShell/GXdgWmBase.h>
#include <protocols/XdgShell/xdg-shell.h>

#include <private/LPositionerPrivate.h>

using namespace Louvre::Protocols::XdgShell;

static struct xdg_positioner_interface xdg_positioner_implementation =
{
    .destroy = &RXdgPositioner::RXdgPositionerPrivate::destroy,
    .set_size = &RXdgPositioner::RXdgPositionerPrivate::set_size,
    .set_anchor_rect = &RXdgPositioner::RXdgPositionerPrivate::set_anchor_rect,
    .set_anchor = &RXdgPositioner::RXdgPositionerPrivate::set_anchor,
    .set_gravity = &RXdgPositioner::RXdgPositionerPrivate::set_gravity,
    .set_constraint_adjustment = &RXdgPositioner::RXdgPositionerPrivate::set_constraint_adjustment,
    .set_offset = &RXdgPositioner::RXdgPositionerPrivate::set_offset,
#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    .set_reactive = &RXdgPositioner::RXdgPositionerPrivate::set_reactive,
    .set_parent_size = &RXdgPositioner::RXdgPositionerPrivate::set_parent_size,
    .set_parent_configure = &RXdgPositioner::RXdgPositionerPrivate::set_parent_configure
#endif
};

RXdgPositioner::RXdgPositioner
(
    GXdgWmBase *gXdgWmBase,
    UInt32 id
)
    :LResource
    (
        gXdgWmBase->client(),
        &xdg_positioner_interface,
        gXdgWmBase->version(),
        id,
        &xdg_positioner_implementation,
        &RXdgPositioner::RXdgPositionerPrivate::destroy_resource
    )
{
    m_imp = new RXdgPositionerPrivate();
    imp()->lPositioner.imp()->data.compositor = compositor();
}

RXdgPositioner::~RXdgPositioner()
{
    delete m_imp;
}

const LPositioner &Louvre::Protocols::XdgShell::RXdgPositioner::positioner() const
{
    return imp()->lPositioner;
}
