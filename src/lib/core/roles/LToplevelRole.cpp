#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <protocols/XdgShell/RXdgSurface.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <private/LSurfacePrivate.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSeatPrivate.h>
#include <LCursor.h>

using namespace Louvre;
using namespace Louvre::Protocols::XdgShell;

LToplevelRole::LToplevelRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        ((LToplevelRole::Params*)params)->toplevel,
        ((LToplevelRole::Params*)params)->surface,
        LSurface::Role::Toplevel)
{
    if (resource()->version() >= 2)
        m_supportedStates.add(TiledLeft | TiledTop | TiledRight | TiledBottom);

    if (resource()->version() >= 6)
        m_supportedStates.add(Suspended);
}

const LToplevelRole::Configuration *LToplevelRole::findConfiguration(UInt32 serial) const noexcept
{
    for (auto &conf : m_sentConfigurations)
        if (conf.serial == serial)
            return &conf;

    if (m_pendingConfiguration.serial == serial)
        return &m_pendingConfiguration;
    if (m_lastACKConfiguration.serial == serial)
        return &m_lastACKConfiguration;

    return nullptr;
}

RXdgToplevel *LToplevelRole::xdgToplevelResource() const
{
    return static_cast<RXdgToplevel*>(resource());
}

RXdgSurface *LToplevelRole::xdgSurfaceResource() const
{
    return xdgToplevelResource()->xdgSurfaceRes();
}

void LToplevelRole::setExclusiveOutput(LOutput *output) noexcept
{
    m_exclusiveOutput.reset(output);

    if (output)
        surface()->sendOutputEnterEvent(output);
}

void LToplevelRole::handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin)
{
    L_UNUSED(origin);

    if (m_flags.check(closedSent))
        return;

    // Configure request
    if (m_flags.check(HasPendingInitialConf))
    {
        if (surface()->hasBuffer())
        {
            wl_resource_post_error(resource()->resource(), XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER, "Attaching a buffer to an unconfigured surface");
            return;
        }

        m_flags.remove(HasPendingInitialConf);
        configureRequest();

        if (m_requestedStateBeforeConf == Fullscreen)
            setFullscreenRequest(m_fullscreenOutputBeforeConf);
        else if (m_requestedStateBeforeConf == Maximized)
            setMaximizedRequest();

        m_requestedStateBeforeConf = NoState;
        m_fullscreenOutputBeforeConf.reset();

        if (!m_flags.check(HasSizeOrStateToSend))
            configureState(pendingConfiguration().state);

        if (!m_flags.check(HasDecorationModeToSend))
            configureDecorationMode(pendingConfiguration().decorationMode);

        if (!m_flags.check(HasBoundsToSend))
            configureBounds(pendingConfiguration().bounds);

        if (!m_flags.check(HasCapabilitiesToSend))
            configureCapabilities(pendingConfiguration().capabilities);

        return;
    }

    // Unmap request
    if (surface()->mapped() && !surface()->hasBuffer())
    {
        reset();
        return;
    }

    fullAtomsUpdate();

    // Map request
    if (!surface()->mapped() && surface()->hasBuffer())
        surface()->imp()->setMapped(true);
}

void LToplevelRole::fullAtomsUpdate()
{
    LBitset<AtomChanges> changesToNotify;

    if (currentAtoms().maxSize != pendingAtoms().maxSize)
        changesToNotify.add(MaxSizeChanged);

    if (currentAtoms().minSize != pendingAtoms().minSize)
        changesToNotify.add(MinSizeChanged);

    if (currentAtoms().serial != pendingAtoms().serial)
    {
        changesToNotify.add(SerialChanged);

        if (currentAtoms().bounds != pendingAtoms().bounds)
            changesToNotify.add(BoundsChanged);

        if (currentAtoms().capabilities != pendingAtoms().capabilities)
            changesToNotify.add(CapabilitiesChanged);

        if (currentAtoms().state != pendingAtoms().state)
        {
            changesToNotify.add(StateChanged);

            const LBitset<State> stateChanges { pendingAtoms().state ^ currentAtoms().state };

            if (stateChanges.check(Activated) && pendingAtoms().state.check(Activated))
            {
                if (seat()->activeToplevel() && seat()->activeToplevel() != this)
                {
                    seat()->activeToplevel()->m_flags.add(ForceRemoveActivatedFlag);
                    seat()->activeToplevel()->configureState(seat()->activeToplevel()->pendingConfiguration().state & ~Activated);
                }

                seat()->imp()->activeToplevel = this;
            }

            if (stateChanges.check(Fullscreen))
            {
                LSurfaceLayer defaultLayer { LLayerMiddle };

                if (surface()->parent())
                    defaultLayer = surface()->parent()->layer();
                else if (surface()->imp()->pendingParent)
                    defaultLayer = surface()->imp()->pendingParent->layer();

                surface()->imp()->setLayer(pendingAtoms().state.check(Fullscreen) ? LLayerTop : defaultLayer);
            }
        }

        if (currentAtoms().decorationMode != pendingAtoms().decorationMode)
            changesToNotify.add(DecorationModeChanged);
    }

    if (xdgSurfaceResource()->m_hasPendingWindowGeometry)
        xdgSurfaceResource()->m_currentWindowGeometry = xdgSurfaceResource()->m_pendingWindowGeometry;
    // If never assigned, use the surface + subsurfaces bounds
    else if (!xdgSurfaceResource()->m_windowGeometrySet)
        xdgSurfaceResource()->m_currentWindowGeometry = xdgSurfaceResource()->calculateGeometryWithSubsurfaces();

    xdgSurfaceResource()->m_hasPendingWindowGeometry = false;

    pendingAtoms().windowGeometry = xdgSurfaceResource()->m_currentWindowGeometry;

    if (currentAtoms().windowGeometry != pendingAtoms().windowGeometry)
        changesToNotify.add(WindowGeometryChanged);

    if (changesToNotify == 0)
        return;

    m_currentAtomsIndex = 1 - m_currentAtomsIndex;

    atomsChanged(changesToNotify, pendingAtoms());

    if (changesToNotify.check(MaxSizeChanged))
        pendingAtoms().maxSize = currentAtoms().maxSize;
    if (changesToNotify.check(MinSizeChanged))
        pendingAtoms().minSize = currentAtoms().minSize;
    if (changesToNotify.check(SerialChanged))
    {
        pendingAtoms().serial = currentAtoms().serial;

        if (changesToNotify.check(BoundsChanged))
            pendingAtoms().bounds = currentAtoms().bounds;

        if (changesToNotify.check(CapabilitiesChanged))
            pendingAtoms().capabilities = currentAtoms().capabilities;

        if (changesToNotify.check(StateChanged))
            pendingAtoms().state = currentAtoms().state;

        if (changesToNotify.check(DecorationModeChanged))
            pendingAtoms().decorationMode = currentAtoms().decorationMode;
    }
    if (changesToNotify.check(WindowGeometryChanged))
    {
        pendingAtoms().windowGeometry = currentAtoms().windowGeometry;
        m_resizeSession.handleGeometryChange();
    }
}

/* Atomic changes that only require ACK but not a surface commit
 * such as Ativated/Suspended state changes, capabilities, etc */
void LToplevelRole::partialAtomsUpdate()
{
    LBitset<AtomChanges> changesToNotify;

    Atoms pendingAtomsBackup { pendingAtoms() };

    pendingAtoms().bounds = currentAtoms().bounds;
    pendingAtoms().maxSize = currentAtoms().maxSize;
    pendingAtoms().minSize = currentAtoms().minSize;
    pendingAtoms().serial = currentAtoms().serial;
    pendingAtoms().windowGeometry = currentAtoms().windowGeometry;
    pendingAtoms().state = currentAtoms().state;

    if (currentAtoms().capabilities != pendingAtoms().capabilities)
        changesToNotify.add(CapabilitiesChanged);

    const LBitset<State> stateChanges { pendingAtomsBackup.state ^ currentAtoms().state };

    if (stateChanges.check(Activated))
    {
        changesToNotify.add(StateChanged);

        if (pendingAtomsBackup.state.check(Activated))
        {
            if (seat()->activeToplevel() && seat()->activeToplevel() != this)
            {
                seat()->activeToplevel()->m_flags.add(ForceRemoveActivatedFlag);
                seat()->activeToplevel()->configureState(seat()->activeToplevel()->pendingConfiguration().state & ~Activated);
            }

            seat()->imp()->activeToplevel = this;
        }

        pendingAtoms().state.setFlag(Activated, pendingAtomsBackup.state.check(Activated));
    }

    if (stateChanges.check(Suspended))
    {
        changesToNotify.add(StateChanged);
        pendingAtoms().state.setFlag(Suspended, pendingAtomsBackup.state.check(Suspended));
    }

    if (stateChanges.check(Resizing))
    {
        changesToNotify.add(StateChanged);
        pendingAtoms().state.setFlag(Resizing, pendingAtomsBackup.state.check(Resizing));
    }

    if (currentAtoms().decorationMode != pendingAtoms().decorationMode)
        changesToNotify.add(DecorationModeChanged);

    m_currentAtomsIndex = 1 - m_currentAtomsIndex;

    atomsChanged(changesToNotify, pendingAtoms());

    // Restore real full pending changes
    pendingAtoms() = pendingAtomsBackup;
}

void LToplevelRole::sendPendingConfiguration() noexcept
{
    if (!m_flags.check(HasSizeOrStateToSend | HasDecorationModeToSend | HasBoundsToSend | HasCapabilitiesToSend))
        return;

    surface()->requestNextFrame(false);

    auto &res { *static_cast<XdgShell::RXdgToplevel*>(resource()) };

    if (m_flags.check(ForceRemoveActivatedFlag))
    {
        m_pendingConfiguration.state.remove(Activated);
        m_flags.remove(ForceRemoveActivatedFlag);
    }

    if (m_flags.check(HasDecorationModeToSend))
    {
        if (m_xdgDecorationRes)
            m_xdgDecorationRes->configure(m_pendingConfiguration.decorationMode);
    }

    if (m_flags.check(HasBoundsToSend))
        res.configureBounds(m_pendingConfiguration.bounds);

    wl_array dummy;

    if (m_flags.check(HasCapabilitiesToSend))
    {
        UInt32 capsArr[4];
        dummy.size = 0;
        dummy.alloc = 0;
        dummy.data = capsArr;

        if (m_pendingConfiguration.capabilities.check(WindowMenuCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU;

        if (m_pendingConfiguration.capabilities.check(MinimizeCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE;

        if (m_pendingConfiguration.capabilities.check(MaximizeCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE;

        if (m_pendingConfiguration.capabilities.check(FullscreenCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN;

        dummy.size = dummy.alloc * sizeof(UInt32);
        dummy.alloc = dummy.size;
        res.wmCapabilities(&dummy);
    }

    UInt32 stateArr[9];
    dummy.size = 0;
    dummy.alloc = 0;
    dummy.data = stateArr;

    if (m_pendingConfiguration.state.check(Activated))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_ACTIVATED;

    if (m_pendingConfiguration.state.check(Fullscreen))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_FULLSCREEN;

    if (m_pendingConfiguration.state.check(Maximized))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_MAXIMIZED;

    if (m_pendingConfiguration.state.check(Resizing))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_RESIZING;

    if (res.version() >= 2)
    {
        if (m_pendingConfiguration.state.check(TiledBottom))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_BOTTOM;

        if (m_pendingConfiguration.state.check(TiledLeft))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_LEFT;

        if (m_pendingConfiguration.state.check(TiledRight))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_RIGHT;

        if (m_pendingConfiguration.state.check(TiledTop))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_TOP;

        if (res.version() >= 6)
        {
            if (m_pendingConfiguration.state.check(Suspended))
                stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_SUSPENDED;
        }
    }

    dummy.size = dummy.alloc * sizeof(UInt32);
    dummy.alloc = dummy.size;

    res.configure(m_pendingConfiguration.size, &dummy);

    if (res.xdgSurfaceRes())
        res.xdgSurfaceRes()->configure(m_pendingConfiguration.serial);

    m_sentConfigurations.push_back(m_pendingConfiguration);

    m_flags.remove(HasSizeOrStateToSend | HasDecorationModeToSend | HasBoundsToSend | HasCapabilitiesToSend);
}

void LToplevelRole::updateSerial() noexcept
{
    if (!m_flags.check(HasSizeOrStateToSend | HasDecorationModeToSend | HasBoundsToSend | HasCapabilitiesToSend))
    {
        m_pendingConfiguration.serial = LTime::nextSerial();
        compositor()->imp()->unlockPoll();
    }
}

void LToplevelRole::reset() noexcept
{
    surface()->imp()->setMapped(false);

    // If a surface becomes unmapped, its children's parent is set to the parent of the now-unmapped surface
    while (!surface()->children().empty())
    {
        if (surface()->children().front()->subsurface())
            surface()->children().front()->imp()->setMapped(false);

        surface()->children().front()->imp()->setParent(surface()->parent());
    }

    surface()->imp()->setParent(nullptr);

    moveSession().stop();
    resizeSession().stop();

    if (!m_title.empty())
    {
        m_title.clear();
        titleChanged();
    }

    if (!m_appId.empty())
    {
        m_appId.clear();
        appIdChanged();
    }

    if (m_preferredDecorationMode != NoPreferredMode)
    {
        m_preferredDecorationMode = NoPreferredMode;
        preferredDecorationModeChanged();
    }

    m_pendingConfiguration = m_lastACKConfiguration = Configuration();
    m_sentConfigurations.clear();
    pendingAtoms() = Atoms();

    if (pendingAtoms().serial == currentAtoms().serial)
        pendingAtoms().serial = LTime::nextSerial();

    fullAtomsUpdate();

    if (seat()->activeToplevel() == this)
        seat()->imp()->activeToplevel = nullptr;

    m_requestedStateBeforeConf = NoState;
    m_fullscreenOutputBeforeConf.reset();

    m_flags = HasPendingInitialConf;
    surface()->imp()->setLayer(LLayerMiddle);
}

void LToplevelRole::close() noexcept
{
    auto &res { *static_cast<XdgShell::RXdgToplevel*>(resource()) };
    res.close();
    m_flags.add(closedSent);
}

LMargins LToplevelRole::calculateConstraintsFromOutput(LOutput *output,  bool includeExtraGeometry) const noexcept
{
    LMargins constraints {LEdgeDisabled, LEdgeDisabled, LEdgeDisabled, LEdgeDisabled};

    if (output)
    {
        if (output->exclusiveEdges().left != 0)
            constraints.left = output->pos().x() + output->exclusiveEdges().left;

        if (output->exclusiveEdges().top != 0)
            constraints.top = output->pos().y() + output->exclusiveEdges().top;

        if (includeExtraGeometry)
        {
            if (output->exclusiveEdges().right != 0)
                constraints.right = output->pos().x() + output->size().w() - output->exclusiveEdges().right - extraGeometry().right - extraGeometry().left;

            if (output->exclusiveEdges().bottom != 0)
                constraints.bottom = output->pos().y() + output->size().h() - output->exclusiveEdges().bottom - extraGeometry().bottom - extraGeometry().top;
        }
        else
        {
            if (output->exclusiveEdges().right != 0)
                constraints.right = output->pos().x() + output->size().w() - output->exclusiveEdges().right;

            if (output->exclusiveEdges().bottom != 0)
                constraints.bottom = output->pos().y() + output->size().h() - output->exclusiveEdges().bottom;
        }
    }

    return constraints;
}
