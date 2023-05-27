#include <protocols/XdgShell/private/RXdgPopupPrivate.h>
#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/private/RXdgPositionerPrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>

#include <protocols/XdgShell/xdg-shell.h>

#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPositionerPrivate.h>

#include <LCompositor.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/RKeyboard.h>

void RXdgPopup::RXdgPopupPrivate::destroy_resource(wl_resource *resource)
{
    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    delete rXdgPopup;
}

void RXdgPopup::RXdgPopupPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    /* This kills Google Chrome
     *
    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    if (rXdgPopup->rXdgSurface() && !rXdgPopup->lPopupRole()->surface()->children().empty())
    {
        wl_resource_post_error(
            rXdgPopup->rXdgSurface()->resource(),
            XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP,
            "The client tried to map or destroy a non-topmost popup.");
    }*/

    wl_resource_destroy(resource);
}

void RXdgPopup::RXdgPopupPrivate::grab(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial)
{
    L_UNUSED(client);
    L_UNUSED(seat);

    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    Wayland::GSeat *lGSeat = (Wayland::GSeat*)wl_resource_get_user_data(seat);

    /* TODO */

    if (true || ( lGSeat->pointerResource() && lGSeat->pointerResource()->serials().button == serial )
        || ( lGSeat->keyboardResource() && lGSeat->keyboardResource()->serials().key == serial))
    {

        LSurface *parent = rXdgPopup->lPopupRole()->surface()->parent();

        if (!parent)
            parent = rXdgPopup->lPopupRole()->surface()->imp()->pendingParent;

        if (!parent || (rXdgPopup->compositor()->seat()->pointer()->focusSurface()
                            != parent && rXdgPopup->compositor()->seat()->keyboard()->focusSurface() != parent))
        {
            wl_resource_post_error(
                resource,
                XDG_POPUP_ERROR_INVALID_GRAB,
                "Invalid grab. Popup parent did not have an implicit grab.");
            return;
        }


        rXdgPopup->lPopupRole()->grabSeatRequest();
    }
    else
    {
        rXdgPopup->lPopupRole()->sendPopupDoneEvent();
    }
}

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
void RXdgPopup::RXdgPopupPrivate::reposition(wl_client *client, wl_resource *resource, wl_resource *positioner, UInt32 token)
{
    L_UNUSED(client);
    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(positioner);

    rXdgPopup->lPopupRole()->imp()->repositionSerial = token;

    rXdgPopup->lPopupRole()->imp()->positioner.imp()->data = rXdgPositioner->positioner().imp()->data;
    rXdgPopup->lPopupRole()->configureRequest();
}
#endif

