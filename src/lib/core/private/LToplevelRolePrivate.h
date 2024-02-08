#ifndef LTOPLEVELROLEPRIVATE_H
#define LTOPLEVELROLEPRIVATE_H

#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LToplevelRole.h>
#include <LCompositor.h>
#include <LTime.h>
#include <queue>

using namespace Louvre;
using namespace Louvre::Protocols;
using namespace Louvre::Protocols::XdgDecoration;

struct LToplevelRole::Params
{
    LResource *toplevel;
    LSurface *surface;
};

LPRIVATE_CLASS(LToplevelRole)
struct ToplevelConfiguration
{
    bool commited                                               { false };
    LSize size                                                  { 0, 0 };
    StateFlags flags                                            { NoState };
    UInt32 serial                                               { 0 };
};

LToplevelRole *toplevel;

bool hasConfToSend { false };
ToplevelConfiguration
    currentConf,        // Current ACKed conf
    pendingApplyConf,   // ACKed conf not yet made the current conf
    pendingSendConf;    // Conf not yet sent to the client
std::list<ToplevelConfiguration>sentConfs;

/* Used to forcefully remove the activated flag when another toplevel is activated */
bool forceRemoveActivatedFlag { false };

bool hasPendingMinSize                                          { false };
bool hasPendingMaxSize                                          { false };

LSize currentMinSize, pendingMinSize;
LSize currentMaxSize, pendingMaxSize;

void setAppId(const char *appId);
void setTitle(const char *title);
std::string appId;
std::string title;

RXdgToplevelDecoration *xdgDecoration                           { nullptr };
DecorationMode decorationMode                                   { ClientSide };
UInt32 pendingDecorationMode                                    { ClientSide };
UInt32 lastDecorationModeConfigureSerial                        { 0 };
LToplevelRole::DecorationMode preferredDecorationMode           { NoPreferredMode };

// Request before the role is applied
StateFlags prevRoleRequest { 0 };
LOutput *prevRoleFullscreenRequestOutput { nullptr };

// Resizing
LPoint resizingInitPos;
LPoint resizingInitPointerPos;
LPoint resizingCurrentPointerPos;
LSize resizingInitWindowSize;
LSize resizingMinSize;
LToplevelRole::ResizeEdge resizingEdge;
LRect resizingConstraintBounds;

inline void applyPendingChanges()
{
    if (!pendingApplyConf.commited)
    {
        pendingApplyConf.commited = true;
        pendingApplyConf.size = toplevel->windowGeometry().size();

        const UInt32 prevState { currentConf.flags };
        currentConf = pendingApplyConf;
        pendingSendConf = currentConf;

        if ((prevState & LToplevelRole::Maximized) != (currentConf.flags & LToplevelRole::Maximized))
            toplevel->maximizedChanged();

        if ((prevState & LToplevelRole::Fullscreen) != (currentConf.flags & LToplevelRole::Fullscreen))
            toplevel->fullscreenChanged();

        if (currentConf.flags & LToplevelRole::Activated)
        {
            if (seat()->activeToplevel() && seat()->activeToplevel() != toplevel)
            {
                seat()->activeToplevel()->imp()->forceRemoveActivatedFlag = true;
                seat()->activeToplevel()->imp()->hasConfToSend = true;
            }

            seat()->imp()->activeToplevel = toplevel;
        }

        if ((prevState & LToplevelRole::Activated) != (currentConf.flags & LToplevelRole::Activated))
            toplevel->activatedChanged();

        if ((prevState & LToplevelRole::Resizing) != (currentConf.flags & LToplevelRole::Resizing))
            toplevel->resizingChanged();

        if (prevState != currentConf.flags)
            toplevel->statesChanged();
    }
}

inline void configure(Int32 width, Int32 height, StateFlags flags)
{
    if (width < 0)
        width = 0;

    if (height < 0)
        height = 0;

    hasConfToSend = true;
    pendingSendConf.size.setW(width);
    pendingSendConf.size.setH(height);
    pendingSendConf.flags = flags;
    compositor()->imp()->unlockPoll();
}

inline void sendConfiguration()
{
    if (!hasConfToSend)
        return;

    hasConfToSend = false;

    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)toplevel->resource();

    toplevel->surface()->requestNextFrame(false);

    pendingSendConf.commited = false;
    pendingSendConf.serial = LTime::nextSerial();

    if (forceRemoveActivatedFlag)
    {
        pendingSendConf.flags = pendingSendConf.flags & ~LToplevelRole::Activated;
        forceRemoveActivatedFlag = false;
    }

    wl_array dummy;
    wl_array_init(&dummy);
    UInt32 index = 0;

    if (pendingSendConf.flags & LToplevelRole::Activated)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_ACTIVATED;
        index++;
    }
    if (pendingSendConf.flags & LToplevelRole::Fullscreen)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_FULLSCREEN;
        index++;
    }
    if (pendingSendConf.flags & LToplevelRole::Maximized)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_MAXIMIZED;
        index++;
    }
    if (pendingSendConf.flags & LToplevelRole::Resizing)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_RESIZING;
        index++;
    }

#if LOUVRE_XDG_WM_BASE_VERSION >= 2
    if (toplevel->resource()->version() >= 2)
    {
        if (pendingSendConf.flags & LToplevelRole::TiledBottom)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_BOTTOM;
            index++;
        }
        if (pendingSendConf.flags & LToplevelRole::TiledLeft)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_LEFT;
            index++;
        }
        if (pendingSendConf.flags & LToplevelRole::TiledRight)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_RIGHT;
            index++;
        }
        if (pendingSendConf.flags & LToplevelRole::TiledTop)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_TOP;
            index++;
        }
    }
#endif

    sentConfs.push_back(pendingSendConf);
    res->configure(pendingSendConf.size.w(), pendingSendConf.size.h(), &dummy);
    wl_array_release(&dummy);

    if (res->xdgSurfaceResource())
    {
        if (pendingDecorationMode != 0 && xdgDecoration)
        {
            xdgDecoration->configure(pendingDecorationMode);
            lastDecorationModeConfigureSerial = pendingSendConf.serial;
        }

        res->xdgSurfaceResource()->configure(pendingSendConf.serial);
    }
}
};

#endif // LTOPLEVELROLEPRIVATE_H
