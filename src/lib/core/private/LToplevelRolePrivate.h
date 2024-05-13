#ifndef LTOPLEVELROLEPRIVATE_H
#define LTOPLEVELROLEPRIVATE_H

#include <LLog.h>
#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LToplevelRole.h>
#include <LCompositor.h>
#include <LTime.h>

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

LBitset<State> supportedStates { Activated | Maximized | Fullscreen | Resizing };

LWeak<RXdgToplevelDecoration> xdgDecoration { nullptr };
DecorationMode preferredDecorationMode { NoPreferredMode };

// Request before the role is applied
LBitset<State> prevRoleRequest { 0 };
LOutput *prevRoleFullscreenRequestOutput { nullptr };

LToplevelResizeSession resizeSession;
LToplevelMoveSession moveSession;

inline void applyPendingChanges(LBitset<ConfigurationChanges> changes)
{
    if (stateFlags.check(HasUncommitedConfiguration))
    {
        stateFlags.remove(HasUncommitedConfiguration);
        previous = current;
        current = uncommited;

        const LBitset<State> stateChanges { previous.state ^ current.state };

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

        if (stateChanges.check(Fullscreen))
        {
            LSurfaceLayer defaultLayer { LLayerMiddle };

            if (toplevel->surface()->parent())
                defaultLayer = toplevel->surface()->parent()->layer();
            else if (toplevel->surface()->imp()->pendingParent)
                defaultLayer = toplevel->surface()->imp()->pendingParent->layer();

            toplevel->surface()->imp()->setLayer(current.state.check(Fullscreen) ? LLayerTop : defaultLayer);
        }

        if (previous.decorationMode != current.decorationMode)
            changes |= DecorationModeChanged;

        if (stateChanges)
            changes |= StateChanged;
    }

    if (changes.check(WindowGeometryChanged))
        resizeSession.handleGeometryChange();

    if (changes)
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

    UInt32 stateArr[8];

    wl_array dummy
    {
        .size = 0,
        .alloc = 0,
        .data = (void*)stateArr
    };

    if (pending.state & LToplevelRole::Activated)
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_ACTIVATED;

    if (pending.state & LToplevelRole::Fullscreen)
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_FULLSCREEN;

    if (pending.state & LToplevelRole::Maximized)
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_MAXIMIZED;

    if (pending.state & LToplevelRole::Resizing)
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_RESIZING;

#if LOUVRE_XDG_WM_BASE_VERSION >= 2
    if (toplevel->resource()->version() >= 2)
    {
        if (pending.state & LToplevelRole::TiledBottom)
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_BOTTOM;

        if (pending.state & LToplevelRole::TiledLeft)
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_LEFT;

        if (pending.state & LToplevelRole::TiledRight)
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_RIGHT;

        if (pending.state & LToplevelRole::TiledTop)
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_TOP;
    }
#endif

    dummy.size = dummy.alloc * sizeof(UInt32);
    dummy.alloc = dummy.size;

    if (pending.decorationMode == NoPreferredMode)
        pending.decorationMode = ClientSide;

    if (xdgDecoration)
    {
        if (current.decorationMode != pending.decorationMode || !stateFlags.check(InitDecorationModeSent))
        {
            xdgDecoration->configure(pending.decorationMode);
            stateFlags.add(InitDecorationModeSent);
        }
    }
    else
        pending.decorationMode = ClientSide;

    xdgToplevelRes.configure(pending.size.w(), pending.size.h(), &dummy);

    if (xdgToplevelRes.xdgSurfaceRes())
        xdgToplevelRes.xdgSurfaceRes()->configure(pending.serial);

    sentConfs.push_back(pending);
}
};

#endif // LTOPLEVELROLEPRIVATE_H
