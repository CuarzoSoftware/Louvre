#include <protocols/Wayland/RKeyboard.h>
#include <protocols/Wayland/RPointer.h>
#include <protocols/Wayland/GSeat.h>
#include <protocols/XdgShell/XdgPopup.h>
#include <protocols/XdgShell/xdg-shell.h>

#include <private/LSurfacePrivate.h>
#include <private/LClientPrivate.h>
#include <private/LPositionerPrivate.h>
#include <private/LPopupRolePrivate.h>

#include <LCompositor.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>

using namespace Louvre;

void Extensions::XdgShell::Popup::destroy_resource(wl_resource *resource)
{
    LPopupRole *lPopup = (LPopupRole*)wl_resource_get_user_data(resource);
    delete lPopup;
}

void Extensions::XdgShell::Popup::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);

    /*
    LPopupRole *lPopup = (LPopupRole*)wl_resource_get_user_data(resource);


    if (!lPopup->surface()->children().empty())
    {

        wl_resource_post_error(
                    lPopup->surface()->client()->xdgWmBaseResource(),
                    XDG_WM_BASE_ERROR_NOT_THE_TOPMOST_POPUP,
                    "The client tried to map or destroy a non-topmost popup.");

    }
    */

    wl_resource_destroy(resource);
}

void Extensions::XdgShell::Popup::grab(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial)
{
    L_UNUSED(seat);
    L_UNUSED(client);

    LPopupRole *lPopup = (LPopupRole*)wl_resource_get_user_data(resource);
    Protocols::Wayland::GSeat *lGSeat = (Protocols::Wayland::GSeat*)wl_resource_get_user_data(seat);

    if (true || ( lGSeat->pointerResource() && lGSeat->pointerResource()->serials().button == serial )
            || ( lGSeat->keyboardResource() && lGSeat->keyboardResource()->serials().key == serial))
    {

        LSurface *parent = lPopup->surface()->parent();

        if (!parent)
            parent = lPopup->surface()->imp()->pendingParent;

        if (!parent || (lPopup->compositor()->seat()->pointer()->focusSurface() != parent && lPopup->compositor()->seat()->keyboard()->focusSurface() != parent))
        {
            wl_resource_post_error(
                        resource,
                        XDG_POPUP_ERROR_INVALID_GRAB,
                        "Invalid grab. Popup parent did not have an implicit grab.");
            return;
        }


        lPopup->grabSeatRequest();
    }
    else
    {
        lPopup->sendPopupDoneEvent();
    }
}

#if LOUVRE_XDG_WM_BASE_VERSION >= 3
void Extensions::XdgShell::Popup::reposition(wl_client *client, wl_resource *resource, wl_resource *positioner, UInt32 serial)
{
    L_UNUSED(client);

    LPositioner *lPositioner = (LPositioner*)wl_resource_get_user_data(positioner);
    LPopupRole *lPopup = (LPopupRole*)wl_resource_get_user_data(resource);
    lPopup->imp()->repositionSerial = serial;

    lPopup->imp()->positioner.imp()->data = lPositioner->imp()->data;
    lPopup->configureRequest();
}
#endif
