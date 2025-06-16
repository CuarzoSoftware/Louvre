#include <CZ/Louvre/Protocols/ForeignToplevelManagement/GForeignToplevelManager.h>
#include <CZ/Louvre/Protocols/ForeignToplevelManagement/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/GForeignToplevelList.h>
#include <CZ/Louvre/Protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgSurface.h>
#include <CZ/Louvre/Protocols/XdgShell/RXdgToplevel.h>
#include <CZ/Louvre/Protocols/XdgShell/xdg-shell.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LToplevelRolePrivate.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <LForeignToplevelController.h>
#include <LClient.h>
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

LToplevelRole::~LToplevelRole()
{
    validateDestructor();
    notifyDestruction();
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

    // Configure request
    if (m_flags.has(HasPendingInitialConf))
    {
        if (surface()->hasBuffer())
        {
            resource()->postError(XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER, "Attaching a buffer to an unconfigured surface");
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

        if (!m_flags.has(HasSizeOrStateToSend))
            configureState(pendingConfiguration().state);

        if (!m_flags.has(HasDecorationModeToSend))
            configureDecorationMode(pendingConfiguration().decorationMode);

        if (!m_flags.has(HasBoundsToSend))
            configureBounds(pendingConfiguration().bounds);

        if (!m_flags.has(HasCapabilitiesToSend))
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
    {
        // ID used by the ext-toplevel-list protocol, must be updated each time
        // the toplevel is re-mapped
        static UInt64 identifier { 0 };
        m_identifier = std::to_string(identifier);
        identifier++;

        for (LClient *client : compositor()->clients())
            for (auto *foreignToplevelList : client->foreignToplevelListGlobals())
                foreignToplevelList->toplevel(*this);

        surface()->imp()->setMapped(true);

        if (m_flags.has(HasPendingFirstMap))
        {
            m_flags.remove(HasPendingFirstMap);

            for (LClient *client : compositor()->clients())
                for (auto *foreignToplevelManager : client->foreignToplevelManagerGlobals())
                    foreignToplevelManager->toplevel(*this);
        }
    }
}

void LToplevelRole::handleParentChange()
{
    LToplevelRole *parent { (surface()->parent() && surface()->parent()->toplevel()) ? surface()->parent()->toplevel() : nullptr };

    if (parent)
    {
        for (auto *controller : m_foreignControllers)
        {
            for (auto *parentController : parent->m_foreignControllers)
            {
                if (controller->resource().foreignToplevelManagerRes() &&
                    controller->resource().foreignToplevelManagerRes() == parentController->resource().foreignToplevelManagerRes())
                {
                    controller->resource().parent(&parentController->resource());
                    controller->resource().done();
                    break;
                }
            }
        }
    }
    else
        for (auto *controller : m_foreignControllers)
            controller->resource().parent(nullptr);
}

void LToplevelRole::fullAtomsUpdate()
{
    CZBitset<AtomChanges> changesToNotify;

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

            const CZBitset<State> stateChanges { pendingAtoms().state ^ currentAtoms().state };

            if (stateChanges.has(Activated) && pendingAtoms().state.has(Activated))
            {
                if (seat()->activeToplevel() && seat()->activeToplevel() != this)
                {
                    seat()->activeToplevel()->m_flags.add(ForceRemoveActivatedFlag);
                    seat()->activeToplevel()->configureState(seat()->activeToplevel()->pendingConfiguration().state & ~Activated);
                }

                seat()->imp()->activeToplevel = this;
            }

            if (stateChanges.has(Fullscreen))
            {
                LSurfaceLayer defaultLayer { LLayerMiddle };

                if (surface()->parent())
                    defaultLayer = surface()->parent()->layer();
                else if (surface()->imp()->pendingParent)
                    defaultLayer = surface()->imp()->pendingParent->layer();

                surface()->imp()->setLayer(pendingAtoms().state.has(Fullscreen) ? LLayerTop : defaultLayer);
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

    const CZBitset<State> stateChanges { pendingAtoms().state ^ currentAtoms().state };

    if (stateChanges.has(Activated | Maximized | Fullscreen))
    {
        for (auto *controller : foreignControllers())
        {
            controller->resource().updateState();
            controller->resource().done();
        }
    }

    if (changesToNotify.has(MaxSizeChanged))
        pendingAtoms().maxSize = currentAtoms().maxSize;
    if (changesToNotify.has(MinSizeChanged))
        pendingAtoms().minSize = currentAtoms().minSize;
    if (changesToNotify.has(SerialChanged))
    {
        pendingAtoms().serial = currentAtoms().serial;

        if (changesToNotify.has(BoundsChanged))
            pendingAtoms().bounds = currentAtoms().bounds;

        if (changesToNotify.has(CapabilitiesChanged))
            pendingAtoms().capabilities = currentAtoms().capabilities;

        if (changesToNotify.has(StateChanged))
            pendingAtoms().state = currentAtoms().state;

        if (changesToNotify.has(DecorationModeChanged))
            pendingAtoms().decorationMode = currentAtoms().decorationMode;
    }
    if (changesToNotify.has(WindowGeometryChanged))
    {
        pendingAtoms().windowGeometry = currentAtoms().windowGeometry;
        m_resizeSession.handleGeometryChange();
    }
}

/* Atomic changes that only require ACK but not a surface commit
 * such as Ativated/Suspended state changes, capabilities, etc */
void LToplevelRole::partialAtomsUpdate()
{
    CZBitset<AtomChanges> changesToNotify;

    Atoms pendingAtomsBackup { pendingAtoms() };

    pendingAtoms().bounds = currentAtoms().bounds;
    pendingAtoms().maxSize = currentAtoms().maxSize;
    pendingAtoms().minSize = currentAtoms().minSize;
    pendingAtoms().serial = currentAtoms().serial;
    pendingAtoms().windowGeometry = currentAtoms().windowGeometry;
    pendingAtoms().state = currentAtoms().state;

    if (currentAtoms().capabilities != pendingAtoms().capabilities)
        changesToNotify.add(CapabilitiesChanged);

    const CZBitset<State> stateChanges { pendingAtomsBackup.state ^ currentAtoms().state };

    if (stateChanges.has(Activated))
    {
        changesToNotify.add(StateChanged);

        if (pendingAtomsBackup.state.has(Activated))
        {
            if (seat()->activeToplevel() && seat()->activeToplevel() != this)
            {
                seat()->activeToplevel()->m_flags.add(ForceRemoveActivatedFlag);
                seat()->activeToplevel()->configureState(seat()->activeToplevel()->pendingConfiguration().state & ~Activated);
            }

            seat()->imp()->activeToplevel = this;
        }

        pendingAtoms().state.setFlag(Activated, pendingAtomsBackup.state.has(Activated));
    }

    if (stateChanges.has(Suspended))
    {
        changesToNotify.add(StateChanged);
        pendingAtoms().state.setFlag(Suspended, pendingAtomsBackup.state.has(Suspended));
    }

    if (stateChanges.has(Resizing))
    {
        changesToNotify.add(StateChanged);
        pendingAtoms().state.setFlag(Resizing, pendingAtomsBackup.state.has(Resizing));
    }

    if (currentAtoms().decorationMode != pendingAtoms().decorationMode)
        changesToNotify.add(DecorationModeChanged);

    m_currentAtomsIndex = 1 - m_currentAtomsIndex;

    atomsChanged(changesToNotify, pendingAtoms());

    if (stateChanges.has(Activated))
    {
        for (auto *controller : foreignControllers())
        {
            controller->resource().updateState();
            controller->resource().done();
        }
    }

    // Restore real full pending changes
    pendingAtoms() = pendingAtomsBackup;
}

void LToplevelRole::sendPendingConfiguration() noexcept
{
    if (!m_flags.has(HasSizeOrStateToSend | HasDecorationModeToSend | HasBoundsToSend | HasCapabilitiesToSend))
        return;

    surface()->requestNextFrame(false);

    auto &res { *static_cast<XdgShell::RXdgToplevel*>(resource()) };

    if (m_flags.has(ForceRemoveActivatedFlag))
    {
        m_pendingConfiguration.state.remove(Activated);
        m_flags.remove(ForceRemoveActivatedFlag);
    }

    if (m_flags.has(HasDecorationModeToSend))
    {
        if (m_xdgDecorationRes)
            m_xdgDecorationRes->configure(m_pendingConfiguration.decorationMode);
    }

    if (m_flags.has(HasBoundsToSend))
        res.configureBounds(m_pendingConfiguration.bounds);

    wl_array dummy;

    if (m_flags.has(HasCapabilitiesToSend))
    {
        UInt32 capsArr[4];
        dummy.size = 0;
        dummy.alloc = 0;
        dummy.data = capsArr;

        if (m_pendingConfiguration.capabilities.has(WindowMenuCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU;

        if (m_pendingConfiguration.capabilities.has(MinimizeCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE;

        if (m_pendingConfiguration.capabilities.has(MaximizeCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE;

        if (m_pendingConfiguration.capabilities.has(FullscreenCap))
            capsArr[dummy.alloc++] = XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN;

        dummy.size = dummy.alloc * sizeof(UInt32);
        dummy.alloc = dummy.size;
        res.wmCapabilities(&dummy);
    }

    UInt32 stateArr[9];
    dummy.size = 0;
    dummy.alloc = 0;
    dummy.data = stateArr;

    if (m_pendingConfiguration.state.has(Activated))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_ACTIVATED;

    if (m_pendingConfiguration.state.has(Fullscreen))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_FULLSCREEN;

    if (m_pendingConfiguration.state.has(Maximized))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_MAXIMIZED;

    if (m_pendingConfiguration.state.has(Resizing))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_RESIZING;

    if (res.version() >= 2)
    {
        if (m_pendingConfiguration.state.has(TiledBottom))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_BOTTOM;

        if (m_pendingConfiguration.state.has(TiledLeft))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_LEFT;

        if (m_pendingConfiguration.state.has(TiledRight))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_RIGHT;

        if (m_pendingConfiguration.state.has(TiledTop))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_TOP;

        if (res.version() >= 6)
        {
            if (m_pendingConfiguration.state.has(Suspended))
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
    if (!m_flags.has(HasSizeOrStateToSend | HasDecorationModeToSend | HasBoundsToSend | HasCapabilitiesToSend))
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

    while (!m_foreignToplevelHandles.empty())
        m_foreignToplevelHandles.back()->closed();
}

void LToplevelRole::setTitle(const char *title)
{
    if (title)
        m_title = title;
    else
        m_title.clear();

    for (auto *controller : m_foreignControllers)
    {
        controller->resource().title(m_title);
        controller->resource().done();
    }

    for (auto *handle : m_foreignToplevelHandles)
    {
        handle->title(m_title);
        handle->done();
    }

    titleChanged();
}

void LToplevelRole::setAppId(const char *appId)
{
    if (appId)
        m_appId = appId;
    else
        m_appId.clear();

    for (auto *controller : m_foreignControllers)
    {
        controller->resource().appId(m_appId);
        controller->resource().done();
    }

    for (auto *handle : m_foreignToplevelHandles)
    {
        handle->appId(m_appId);
        handle->done();
    }

    appIdChanged();
}

void LToplevelRole::close() noexcept
{
    auto &res { *static_cast<XdgShell::RXdgToplevel*>(resource()) };
    res.close();
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
                constraints.right = output->pos().x() + output->size().width() - output->exclusiveEdges().right - extraGeometry().right - extraGeometry().left;

            if (output->exclusiveEdges().bottom != 0)
                constraints.bottom = output->pos().y() + output->size().height() - output->exclusiveEdges().bottom - extraGeometry().bottom - extraGeometry().top;
        }
        else
        {
            if (output->exclusiveEdges().right != 0)
                constraints.right = output->pos().x() + output->size().width() - output->exclusiveEdges().right;

            if (output->exclusiveEdges().bottom != 0)
                constraints.bottom = output->pos().y() + output->size().height() - output->exclusiveEdges().bottom;
        }
    }

    return constraints;
}
