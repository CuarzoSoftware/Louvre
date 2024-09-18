#include <private/LXWindowRolePrivate.h>
#include <private/LXWaylandPrivate.h>
#include <private/LSurfacePrivate.h>
#include <xcb/xcb_icccm.h>

void LXWindowRole::LXWindowRolePrivate::setOverrideRedirect(bool override)
{
    if (overrideRedirect == override)
        return;

    overrideRedirect = override;
    xWayland()->imp()->updateNetClientListStacking();

    // TODO: Update stacking
}

void LXWindowRole::LXWindowRolePrivate::updateWMState() noexcept
{
    static UInt32 property[] { XCB_ICCCM_WM_STATE_NORMAL, XCB_WINDOW_NONE };

    if (withdrawn)
        property[0] = XCB_ICCCM_WM_STATE_WITHDRAWN;
    else if (minimized)
        property[0] = XCB_ICCCM_WM_STATE_ICONIC;
    else
        property[0] = XCB_ICCCM_WM_STATE_NORMAL;

    auto &x { *xWayland()->imp() };

    xcb_change_property(x.conn,
                        XCB_PROP_MODE_REPLACE,
                        winId,
                        x.atoms[WM_STATE],
                        x.atoms[WM_STATE],
                        32,
                        sizeof(property) / sizeof(property[0]),
                        property);
}

void LXWindowRole::LXWindowRolePrivate::updateNetWMState() noexcept
{
    auto &x { *xWayland()->imp() };

    if (withdrawn)
    {
        xcb_delete_property(x.conn, winId, x.atoms[NET_WM_STATE]);
        return;
    }

    UInt32 props[6];
    size_t n { 0 };

    if (modal)
        props[n++] = x.atoms[NET_WM_STATE_MODAL];

    if (fullscreen)
        props[n++] = x.atoms[NET_WM_STATE_FULLSCREEN];

    if (maximizedY)
        props[n++] = x.atoms[NET_WM_STATE_MAXIMIZED_VERT];

    if (maximizedX)
        props[n++] = x.atoms[NET_WM_STATE_MAXIMIZED_HORZ];

    if (minimized)
        props[n++] = x.atoms[NET_WM_STATE_HIDDEN];


    /*TODO if (xsurface == xwm->focus_surface) {
        props[n++] = x.atoms[NET_WM_STATE_FOCUSED];
    }*/

    xcb_change_property(x.conn,
                        XCB_PROP_MODE_REPLACE,
                        winId,
                        x.atoms[NET_WM_STATE],
                        XCB_ATOM_ATOM,
                        32,
                        n, props);
}

void LXWindowRole::LXWindowRolePrivate::setWithdrawn(bool withdrawn) noexcept
{
    this->withdrawn = withdrawn;
    updateWMState();
    updateNetWMState();
    xcb_flush(xWayland()->imp()->conn);
}
