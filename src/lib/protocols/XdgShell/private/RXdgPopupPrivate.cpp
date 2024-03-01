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
#include <LSeat.h>
#include <LKeyboard.h>
#include <LClient.h>
#include <LLog.h>

void RXdgPopup::RXdgPopupPrivate::destroy(wl_client *client, wl_resource *resource)
{
    L_UNUSED(client);
    wl_resource_destroy(resource);
}

void RXdgPopup::RXdgPopupPrivate::grab(wl_client *client, wl_resource *resource, wl_resource *seat, UInt32 serial)
{
    L_UNUSED(client);
    L_UNUSED(seat);

    RXdgPopup *rXdgPopup { (RXdgPopup*)wl_resource_get_user_data(resource) };

    if (!rXdgPopup->popupRole()->surface())
    {
        LLog::warning("[RXdgPopup::RXdgPopupPrivate::grab] XDG Popup keyboard grab request without surface. Ignoring it.");
        return;
    }

    const LEvent *triggeringEvent { rXdgPopup->client()->findEventBySerial(serial) };

    if (!triggeringEvent)
    {
        LLog::warning("[RXdgPopup::RXdgPopupPrivate::grab] XDG Popup keyboard grab request without valid event serial. Ignoring it.");
        rXdgPopup->popupRole()->dismiss();
        return;
    }

    rXdgPopup->popupRole()->grabKeyboardRequest(*triggeringEvent);

    // Check if the user accepted the grab
    if (compositor()->seat()->keyboard()->grab() != rXdgPopup->popupRole()->surface())
        rXdgPopup->popupRole()->dismiss();
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
