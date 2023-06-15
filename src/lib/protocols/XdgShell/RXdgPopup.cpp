#include <protocols/XdgShell/private/RXdgPopupPrivate.h>
#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/private/RXdgPositionerPrivate.h>

#include <protocols/XdgShell/xdg-shell.h>

#include <private/LPointerPrivate.h>
#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>

#include <LCompositor.h>

static struct xdg_popup_interface xdg_popup_implementation =
{
    .destroy = &RXdgPopup::RXdgPopupPrivate::destroy,
    .grab = &RXdgPopup::RXdgPopupPrivate::grab,

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    .reposition = &RXdgPopup::RXdgPopupPrivate::reposition
#endif
};

RXdgPopup::RXdgPopup
(
    RXdgSurface *rXdgSurface,
    RXdgSurface *rXdgParentSurface,
    RXdgPositioner *rXdgPositioner,
    UInt32 id
)
    :LResource
    (
        rXdgSurface->client(),
        &xdg_popup_interface,
        rXdgSurface->version(),
        id,
        &xdg_popup_implementation,
        &RXdgPopup::RXdgPopupPrivate::destroy_resource
    )
{
    m_imp = new RXdgPopupPrivate();
    imp()->rXdgSurface = rXdgSurface;
    rXdgSurface->imp()->rXdgPopup = this;

    rXdgSurface->lSurface()->imp()->setPendingParent(rXdgParentSurface->lSurface());

    LPopupRole::Params popupRoleParams;
    popupRoleParams.popup = this;
    popupRoleParams.surface = rXdgSurface->lSurface();
    popupRoleParams.positioner = &rXdgPositioner->imp()->lPositioner;

    imp()->lPopupRole = compositor()->createPopupRoleRequest(&popupRoleParams);

    rXdgSurface->lSurface()->imp()->setPendingRole(imp()->lPopupRole);
}

RXdgPopup::~RXdgPopup()
{
    if (rXdgSurface())
        rXdgSurface()->imp()->rXdgPopup = nullptr;

    delete imp()->lPopupRole;
    delete m_imp;
}

RXdgSurface *RXdgPopup::rXdgSurface() const
{
    return imp()->rXdgSurface;
}

LPopupRole *RXdgPopup::lPopupRole() const
{
    return imp()->lPopupRole;
}

bool RXdgPopup::configure(Int32 x, Int32 y, Int32 width, Int32 height) const
{
    xdg_popup_send_configure(resource(), x, y, width, height);
    return true;
}

bool RXdgPopup::popup_done() const
{
    xdg_popup_send_popup_done(resource());
    return true;
}

bool RXdgPopup::repositioned(UInt32 token) const
{
#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    if (version() >= 3)
    {
        xdg_popup_send_repositioned(resource(), token);
        return true;
    }
#endif
    L_UNUSED(token);
    return false;
}
