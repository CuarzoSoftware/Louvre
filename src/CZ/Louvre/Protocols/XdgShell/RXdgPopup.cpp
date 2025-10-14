#include <CZ/Louvre/Protocols/XdgShell/xdg-shell.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgPopup.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgSurface.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgPositioner.h>
#include <CZ/Louvre/Private/LPointerPrivate.h>
#include <CZ/Louvre/Private/LPopupRolePrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ::Protocols::XdgShell;

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

    m_popupRole.reset(LFactory::createObject<LPopupRole>(&popupRoleParams));

    xdgSurfaceRes->surface()->imp()->notifyRoleChange();

    if (xdgParentSurfaceRes)
    {
        assert(xdgParentSurfaceRes->surface());
        m_popupRole->setParent(xdgParentSurfaceRes->surface());
    }
}

RXdgPopup::~RXdgPopup()
{
    compositor()->onAnticipatedObjectDestruction(popupRole());

    if (!popupRole()->childPopups().empty())
    {
        xdgSurfaceRes()->postError(XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP, "The client tried to map or destroy a non-topmost popup");
        return;
    }

    popupRole()->surface()->imp()->setKeyboardGrabToParent();
    popupRole()->surface()->imp()->setMapped(false);
    popupRole()->setParent(nullptr);
    popupRole()->surface()->imp()->setRole(nullptr, true);
}

/******************** REQUESTS ********************/

void RXdgPopup::destroy(wl_client */*client*/, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RXdgPopup::grab(wl_client */*client*/, wl_resource *resource, wl_resource */*seat*/, UInt32 serial)
{
    auto &res { *static_cast<RXdgPopup*>(wl_resource_get_user_data(resource)) };

    if (!res.popupRole()->surface()->mapped() || res.popupRole()->m_flags.has(LPopupRole::Dismissed))
        return;

    const CZEvent *triggeringEvent { res.client()->findEventBySerial(serial) };

    if (!triggeringEvent)
    {
        LLog(CZWarning, CZLN, "Grab request without valid event serial. Ignoring it...");
        res.popupRole()->dismiss();
        return;
    }

    res.popupRole()->grabKeyboardRequest(*triggeringEvent);

    // Check if the user accepted the grab
    if (seat()->keyboard()->grab() != res.popupRole()->surface())
        res.popupRole()->dismiss();
}

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
void RXdgPopup::reposition(wl_client */*client*/, wl_resource *resource, wl_resource *positioner, UInt32 token)
{
    RXdgPopup &res { *static_cast<RXdgPopup*>(wl_resource_get_user_data(resource)) };

    /* TODO Report bug: Some GTK4 apps trigger reposition requests before the popup is mapped */

    /* if (!res.popupRole()->surface()->mapped())
        return; */

    RXdgPositioner &xdgPositionerRes { *static_cast<RXdgPositioner*>(wl_resource_get_user_data(positioner)) };

    if (!xdgPositionerRes.validate())
        return;

    res.popupRole()->m_positioner = xdgPositionerRes.m_positioner;
    res.popupRole()->m_repositionToken = token;
    res.popupRole()->m_flags.add(LPopupRole::HasPendingReposition | LPopupRole::CanBeConfigured);
    res.popupRole()->configureRequest();
    if (!res.popupRole()->m_flags.has(LPopupRole::HasConfigurationToSend))
        res.popupRole()->configureRect(res.popupRole()->calculateUnconstrainedRect());
}
#endif

/******************** EVENTS ********************/

void RXdgPopup::configure(const SkIRect &rect) noexcept
{
    xdg_popup_send_configure(resource(), rect.x(), rect.y(), rect.width(), rect.height());
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
    CZ_UNUSED(token);
    return false;
}
