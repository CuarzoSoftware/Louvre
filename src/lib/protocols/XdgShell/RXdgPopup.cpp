#include <protocols/XdgShell/RXdgPopup.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgPositioner.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LPointerPrivate.h>
#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LKeyboard.h>
#include <LCompositor.h>
#include <LClient.h>
#include <LLog.h>

using namespace Louvre::Protocols::XdgShell;

static const struct xdg_popup_interface imp
{
    .destroy = &RXdgPopup::destroy,
    .grab = &RXdgPopup::grab,
#if LOUVRE_XDG_WM_BASE_VERSION >= 3
    .reposition = &RXdgPopup::reposition
#else
    .reposition = NULL
#endif
};

RXdgPopup::RXdgPopup
(
    RXdgSurface *xdgSurfaceRes,
    RXdgSurface *xdgParentSurfaceRes,
    RXdgPositioner *xdgPositionerRes,
    UInt32 id
)
    :LResource
    (
        xdgSurfaceRes->client(),
        &xdg_popup_interface,
        xdgSurfaceRes->version(),
        id,
        &imp
    ),
    m_xdgSurfaceRes(xdgSurfaceRes)
{
    xdgSurfaceRes->m_xdgPopupRes.reset(this);

    LPopupRole::Params popupRoleParams
    {
        this,
        xdgSurfaceRes->surface(),
        &xdgPositionerRes->m_positioner
    };

    m_popupRole.reset(compositor()->createPopupRoleRequest(&popupRoleParams));
    xdgSurfaceRes->surface()->imp()->setParent(xdgParentSurfaceRes->surface());
    xdgSurfaceRes->surface()->imp()->setPendingRole(popupRole());
}

RXdgPopup::~RXdgPopup()
{
    compositor()->destroyPopupRoleRequest(popupRole());

    if (popupRole()->surface())
    {
        for (LSurface *child : popupRole()->surface()->children())
        {
            if (child->popup() && child->mapped())
            {
                wl_resource_post_error(
                    xdgSurfaceRes()->resource(),
                    XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP,
                    "The client tried to map or destroy a non-topmost popup.");
            }
        }

        popupRole()->surface()->imp()->setKeyboardGrabToParent();
        popupRole()->surface()->imp()->setMapped(false);
    }
}

/******************** REQUESTS ********************/

void RXdgPopup::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RXdgPopup::grab(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial)
{
    auto &res { *static_cast<RXdgPopup*>(wl_resource_get_user_data(resource)) };

    if (!res.popupRole()->surface())
    {
        LLog::warning("[RXdgPopup::grab] XDG Popup keyboard grab request without surface. Ignoring it.");
        return;
    }

    const LEvent *triggeringEvent { res.client()->findEventBySerial(serial) };

    if (!triggeringEvent)
    {
        LLog::warning("[RXdgPopup::grab] XDG Popup keyboard grab request without valid event serial. Ignoring it.");
        res.popupRole()->dismiss();
        return;
    }

    // TODO use LWeak
    res.popupRole()->grabKeyboardRequest(*triggeringEvent);

    // Check if the user accepted the grab
    if (seat()->keyboard()->grab() != res.popupRole()->surface())
        res.popupRole()->dismiss();
}

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
void RXdgPopup::reposition(wl_client *client, wl_resource *resource, wl_resource *positioner, UInt32 token)
{
    L_UNUSED(client);
    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(positioner);
    rXdgPopup->popupRole()->imp()->positioner.imp()->data = rXdgPositioner->positioner().imp()->data;
    rXdgPopup->popupRole()->repositionRequest(token);
}
#endif

/******************** EVENTS ********************/

void RXdgPopup::configure(Int32 x, Int32 y, Int32 width, Int32 height) noexcept
{
    xdg_popup_send_configure(resource(), x, y, width, height);
}

void RXdgPopup::popupDone() noexcept
{
    xdg_popup_send_popup_done(resource());
}

bool RXdgPopup::repositioned(UInt32 token) noexcept
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
