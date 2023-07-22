#include <protocols/XdgShell/private/RXdgPopupPrivate.h>
#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/private/RXdgPositionerPrivate.h>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/RKeyboard.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LPopupRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LPositionerPrivate.h>
#include <LCompositor.h>

void RXdgPopup::RXdgPopupPrivate::destroy_resource(wl_resource *resource)
{
    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    delete rXdgPopup;
}

void RXdgPopup::RXdgPopupPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);

    if (rXdgPopup->popupRole()->surface())
    {
        for (LSurface *child : rXdgPopup->popupRole()->surface()->children())
        {
            if (child->popup() && child->mapped())
            {
                wl_resource_post_error(
                    rXdgPopup->xdgSurfaceResource()->resource(),
                    XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP,
                    "The client tried to map or destroy a non-topmost popup.");
            }
        }

        rXdgPopup->popupRole()->surface()->imp()->setKeyboardGrabToParent();
    }

    wl_resource_destroy(resource);
}

void RXdgPopup::RXdgPopupPrivate::grab(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial)
{
    L_UNUSED(client);
    L_UNUSED(seat);

    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    Wayland::GSeat *lGSeat = (Wayland::GSeat*)wl_resource_get_user_data(seat);

    if ((lGSeat->pointerResource() && (lGSeat->pointerResource()->serials().button >= serial || lGSeat->pointerResource()->serials().enter == serial)) ||
        (lGSeat->keyboardResource() && (lGSeat->keyboardResource()->serials().key >= serial || lGSeat->keyboardResource()->serials().enter == serial)))
    {
        LSurface *parent = rXdgPopup->popupRole()->surface()->parent();

        if (!parent)
            parent = rXdgPopup->popupRole()->surface()->imp()->pendingParent;

        if (!parent || (compositor()->seat()->pointer()->focusSurface() != parent && compositor()->seat()->keyboard()->focusSurface() != parent))
        {
            /* TODO: Fix grab.
            wl_resource_post_error(
                resource,
                XDG_POPUP_ERROR_INVALID_GRAB,
                "Invalid grab. Popup parent did not have an implicit grab."); */
            return;
        }

        rXdgPopup->popupRole()->grabSeatRequest(lGSeat);
    }
    else
        rXdgPopup->popupRole()->sendPopupDoneEvent();
}

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
void RXdgPopup::RXdgPopupPrivate::reposition(wl_client *client, wl_resource *resource, wl_resource *positioner, UInt32 token)
{
    L_UNUSED(client);
    RXdgPopup *rXdgPopup = (RXdgPopup*)wl_resource_get_user_data(resource);
    RXdgPositioner *rXdgPositioner = (RXdgPositioner*)wl_resource_get_user_data(positioner);
    rXdgPopup->popupRole()->imp()->positioner.imp()->data = rXdgPositioner->positioner().imp()->data;
    rXdgPopup->popupRole()->repositionRequest(token);
}
#endif
