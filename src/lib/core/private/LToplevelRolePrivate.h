#ifndef LTOPLEVELROLEPRIVATE_H
#define LTOPLEVELROLEPRIVATE_H

#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <roles/LToplevelMoveSession.h>
#include <roles/LToplevelResizeSession.h>
#include <roles/LToplevelRole.h>
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

enum StateFlags : UInt8
{
    HasConfigurationToSend      = static_cast<UInt8>(1) << 0,
    HasUncommitedConfiguration  = static_cast<UInt8>(1) << 1,
    HasPendingMinSize           = static_cast<UInt8>(1) << 2,
    HasPendingMaxSize           = static_cast<UInt8>(1) << 3,
    ForceRemoveActivatedFlag    = static_cast<UInt8>(1) << 4,
    InitDecorationModeSent      = static_cast<UInt8>(1) << 5,
};

LToplevelRole *toplevel;
LBitset<StateFlags> stateFlags;
Configuration current, pending, previous, uncommited;
std::list<Configuration> sentConfs;

LSize currentMinSize, pendingMinSize;
LSize currentMaxSize, pendingMaxSize;

void setAppId(const char *appId);
void setTitle(const char *title);
std::string appId;
std::string title;

LWeak<RXdgToplevelDecoration> xdgDecoration { nullptr };
DecorationMode preferredDecorationMode { NoPreferredMode };

// Request before the role is applied
LBitset<State> prevRoleRequest { 0 };
LOutput *prevRoleFullscreenRequestOutput { nullptr };

LToplevelResizeSession resizeSession;
LToplevelMoveSession moveSession;

inline void applyPendingChanges(LBitset<ConfigurationChanges> changes)
{
    if (!stateFlags.check(HasUncommitedConfiguration))
        return;

    stateFlags.remove(HasUncommitedConfiguration);
    previous = current;
    current = uncommited;

    const LBitset<State> stateChanges { static_cast<State>((previous.state.get() ^ current.state.get())) };

    if (stateChanges.check(Activated))
    {
        if (current.state.check(Activated))
        {
            if (seat()->activeToplevel() && seat()->activeToplevel() != toplevel)
            {
                seat()->activeToplevel()->imp()->stateFlags.add(ForceRemoveActivatedFlag);

                if (!seat()->activeToplevel()->imp()->stateFlags.check(HasConfigurationToSend))
                {
                    seat()->activeToplevel()->imp()->pending.serial = LTime::nextSerial();
                    seat()->activeToplevel()->imp()->stateFlags.add(HasConfigurationToSend);
                }
            }

            seat()->imp()->activeToplevel = toplevel;
        }
    }

    if (previous.decorationMode != current.decorationMode)
        changes |= DecorationModeChanged;

    if (stateChanges)
        changes |= StateChanged;

    toplevel->configurationChanged(changes);
}

inline void sendConfiguration()
{
    if (!stateFlags.check(HasConfigurationToSend))
        return;

    stateFlags.remove(HasConfigurationToSend);

    auto &xdgToplevelRes { *static_cast<XdgShell::RXdgToplevel*>(toplevel->resource()) };
    toplevel->surface()->requestNextFrame(false);

    if (stateFlags.check(ForceRemoveActivatedFlag))
    {
        pending.state.remove(LToplevelRole::Activated);
        stateFlags.remove(ForceRemoveActivatedFlag);
    }

    wl_array dummy;
    wl_array_init(&dummy);
    UInt32 index = 0;

    if (pending.state & LToplevelRole::Activated)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_ACTIVATED;
        index++;
    }
    if (pending.state & LToplevelRole::Fullscreen)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_FULLSCREEN;
        index++;
    }
    if (pending.state & LToplevelRole::Maximized)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_MAXIMIZED;
        index++;
    }
    if (pending.state & LToplevelRole::Resizing)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_RESIZING;
        index++;
    }

#if LOUVRE_XDG_WM_BASE_VERSION >= 2
    if (toplevel->resource()->version() >= 2)
    {
        if (pending.state & LToplevelRole::TiledBottom)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_BOTTOM;
            index++;
        }
        if (pending.state & LToplevelRole::TiledLeft)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_LEFT;
            index++;
        }
        if (pending.state & LToplevelRole::TiledRight)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_RIGHT;
            index++;
        }
        if (pending.state & LToplevelRole::TiledTop)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_TOP;
            index++;
        }
    }
#endif

    if (pending.decorationMode == NoPreferredMode)
        pending.decorationMode = ClientSide;

    if (xdgDecoration.get())
    {
        if (current.decorationMode != pending.decorationMode || !stateFlags.check(InitDecorationModeSent))
        {
            xdgDecoration.get()->configure(pending.decorationMode);
            stateFlags.add(InitDecorationModeSent);
        }
    }
    else
        pending.decorationMode = ClientSide;

    xdgToplevelRes.configure(pending.size.w(), pending.size.h(), &dummy);
    wl_array_release(&dummy);

    if (xdgToplevelRes.xdgSurfaceRes())
        xdgToplevelRes.xdgSurfaceRes()->configure(pending.serial);

    sentConfs.push_back(pending);
}
};

#endif // LTOPLEVELROLEPRIVATE_H
