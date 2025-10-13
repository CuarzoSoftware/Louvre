#include "LLog.h"
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
#include <CZ/Louvre/Roles/LForeignToplevelController.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Cursor/LCursor.h>

using namespace CZ;
using namespace CZ::Protocols::XdgShell;

LToplevelRole::LToplevelRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        ((LToplevelRole::Params*)params)->toplevel,
        ((LToplevelRole::Params*)params)->surface,
        LSurface::Role::Toplevel)
{
    if (resource()->version() >= 2)
        m_supportedWindowStates.add(CZWinTiledLeft | CZWinTiledTop | CZWinTiledRight | CZWinTiledBottom);

    if (resource()->version() >= 6)
        m_supportedWindowStates.add(CZWinSuspended);
}

LToplevelRole::~LToplevelRole() noexcept
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

void LToplevelRole::setMinimized(bool minimized) noexcept
{
    if (minimized == isMinimized())
        return;

    m_flags.setFlag(IsMinimized, minimized);

    for (auto *controller : m_foreignControllers)
    {
        controller->resource().updateState();
        controller->resource().done();
    }

    minimizedChanged();
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

void LToplevelRole::sendPendingConfiguration() noexcept
{
    if (!m_flags.has(HasSizeOrStateToSend | HasDecorationModeToSend | HasBoundsToSend | HasCapabilitiesToSend))
        return;

    surface()->requestNextFrame(false);

    auto &res { *static_cast<XdgShell::RXdgToplevel*>(resource()) };

    if (m_flags.has(ForceRemoveActivatedFlag))
    {
        m_pendingConfiguration.windowState.remove(CZWinActivated);
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

    if (m_pendingConfiguration.windowState.has(CZWinActivated))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_ACTIVATED;

    if (m_pendingConfiguration.windowState.has(CZWinFullscreen))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_FULLSCREEN;

    if (m_pendingConfiguration.windowState.has(CZWinMaximized))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_MAXIMIZED;

    if (m_pendingConfiguration.windowState.has(CZWinResizing))
        stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_RESIZING;

    if (res.version() >= 2)
    {
        if (m_pendingConfiguration.windowState.has(CZWinTiledBottom))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_BOTTOM;

        if (m_pendingConfiguration.windowState.has(CZWinTiledLeft))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_LEFT;

        if (m_pendingConfiguration.windowState.has(CZWinTiledRight))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_RIGHT;

        if (m_pendingConfiguration.windowState.has(CZWinTiledTop))
            stateArr[dummy.alloc++] = XDG_TOPLEVEL_STATE_TILED_TOP;

        if (res.version() >= 6)
        {
            if (m_pendingConfiguration.windowState.has(CZWinSuspended))
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
        m_pendingConfiguration.serial = CZTime::NextSerial();
        compositor()->imp()->unlockPoll();
    }
}

void LToplevelRole::reset(CZ::LToplevelRole::State *pending) noexcept
{
    surface()->imp()->setMapped(false);

    auto *parent { surface()->parent() ? surface()->parent()->toplevel() : nullptr };

    while (!m_childToplevels.empty())
        m_childToplevels.front()->setParent(parent);

    setParent(nullptr);

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

    *pending = {};

    while (pending->serial == m_current.serial)
        pending->serial = CZTime::NextSerial();

    applyState(pending);

    if (seat()->activeToplevel() == this)
        seat()->imp()->setActiveToplevel(nullptr);

    m_requestedStateBeforeConf = CZWinNoState;
    m_fullscreenOutputBeforeConf.reset();

    m_flags = HasPendingInitialConf;

    while (!m_foreignToplevelHandles.empty())
        m_foreignToplevelHandles.back()->closed();
}

void LToplevelRole::setTitle(const char *title) noexcept
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

void LToplevelRole::setAppId(const char *appId) noexcept
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

void LToplevelRole::setParent(LToplevelRole *parent) noexcept
{
    auto *newParent { parent && parent->surface()->mapped() ? parent : nullptr };
    auto *oldParent { surface()->parent() ? surface()->parent()->toplevel() : nullptr };

    if (newParent == oldParent)
        return;

    if (oldParent)
        oldParent->m_childToplevels.erase(m_parentLink);

    if (newParent)
    {
        newParent->m_childToplevels.emplace_back(this);
        m_parentLink = std::prev(newParent->m_childToplevels.end());
        surface()->imp()->setParent(newParent->surface());
    }
    else
        surface()->imp()->setParent(nullptr);

    handleParentChange();
}

void LToplevelRole::handleParentChange() noexcept
{
    auto *parent { surface()->parent() ? surface()->parent()->toplevel() : nullptr };

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


void LToplevelRole::close() noexcept
{
    auto &res { *static_cast<XdgShell::RXdgToplevel*>(resource()) };
    res.close();
}

LMargins LToplevelRole::calculateConstraintsFromOutput(LOutput *output) const noexcept
{
    LMargins constraints { CZEdgeDisabled, CZEdgeDisabled, CZEdgeDisabled, CZEdgeDisabled };

    if (output)
    {
        if (output->exclusiveEdges().left != 0)
            constraints.left = output->pos().x() + output->exclusiveEdges().left;

        if (output->exclusiveEdges().top != 0)
            constraints.top = output->pos().y() + output->exclusiveEdges().top;

        if (output->exclusiveEdges().right != 0)
            constraints.right = output->pos().x() + output->size().width() - output->exclusiveEdges().right;

        if (output->exclusiveEdges().bottom != 0)
            constraints.bottom = output->pos().y() + output->size().height() - output->exclusiveEdges().bottom;
    }

    return constraints;
}

void LToplevelRole::cacheCommit() noexcept
{
    m_pending.windowGeometry = xdgSurfaceResource()->m_windowGeometry;
    m_pending.windowGeometrySet = xdgSurfaceResource()->m_windowGeometrySet;

    if (surface()->isLocked())
        m_cache.emplace_back(std::shared_ptr<State>(new State(m_pending)));
}

void LToplevelRole::applyCommit() noexcept
{
    std::shared_ptr<State> cachedPending;
    State *pending;

    if (m_cache.empty())
        pending = &m_pending;
    else
    {
        cachedPending = m_cache.front();
        m_cache.pop_front();
        pending = cachedPending.get();
    }

    // Configure request
    if (m_flags.has(HasPendingInitialConf))
    {
        if (surface()->bufferResource())
        {
            resource()->postError(XDG_SURFACE_ERROR_UNCONFIGURED_BUFFER, "Attaching a buffer to an unconfigured surface");
            return;
        }

        m_flags.remove(HasPendingInitialConf);
        configureRequest();

        if (m_requestedStateBeforeConf == CZWinFullscreen)
            setFullscreenRequest(m_fullscreenOutputBeforeConf);
        else if (m_requestedStateBeforeConf == CZWinMaximized)
            setMaximizedRequest();

        m_requestedStateBeforeConf = CZWinNoState;
        m_fullscreenOutputBeforeConf.reset();

        if (!m_flags.has(HasSizeOrStateToSend))
            configureState(pendingConfiguration().windowState);

        if (!m_flags.has(HasDecorationModeToSend))
            configureDecorationMode(pendingConfiguration().decorationMode);

        if (!m_flags.has(HasBoundsToSend))
            configureBounds(pendingConfiguration().bounds);

        if (!m_flags.has(HasCapabilitiesToSend))
            configureCapabilities(pendingConfiguration().capabilities);

        return;
    }

    // Unmap request
    if (surface()->mapped() && !surface()->bufferResource())
    {
        reset(pending);
        return;
    }

    applyState(pending);

    // Map request
    if (!surface()->mapped() && surface()->bufferResource())
    {
        // ID used by the ext-toplevel-list protocol, must be updated each time
        // the toplevel is re-mapped
        static UInt64 identifier { 0 };
        m_identifier = std::to_string(identifier);
        identifier++;

        for (LClient *client : compositor()->clients())
            for (auto *foreignToplevelList : client->foreignToplevelListGlobals())
                foreignToplevelList->toplevel(*this);

        surface()->imp()->setMapped(true, true);

        if (m_flags.has(HasPendingFirstMap))
        {
            m_flags.remove(HasPendingFirstMap);

            for (LClient *client : compositor()->clients())
                for (auto *foreignToplevelManager : client->foreignToplevelManagerGlobals())
                    foreignToplevelManager->toplevel(*this);
        }
    }
}

void LToplevelRole::applyState(State *pending) noexcept
{
    CZBitset<Changes> changesToNotify;

    if (m_current.maxSize != pending->maxSize)
        changesToNotify.add(MaxSizeChanged);

    if (m_current.minSize != pending->minSize)
        changesToNotify.add(MinSizeChanged);

    if (m_current.serial != pending->serial)
    {
        changesToNotify.add(SerialChanged);

        if (m_current.bounds != pending->bounds)
            changesToNotify.add(BoundsChanged);

        if (m_current.capabilities != pending->capabilities)
            changesToNotify.add(CapabilitiesChanged);

        if (m_current.windowState != pending->windowState)
        {
            changesToNotify.add(StateChanged);

            const CZBitset<CZWindowState> stateChanges { pending->windowState ^ m_current.windowState };

            if (stateChanges.has(CZWinActivated) && pending->windowState.has(CZWinActivated))
            {
                if (seat()->activeToplevel() && seat()->activeToplevel() != this)
                {
                    seat()->activeToplevel()->m_flags.add(ForceRemoveActivatedFlag);
                    seat()->activeToplevel()->configureState(seat()->activeToplevel()->pendingConfiguration().windowState & ~CZWinActivated);
                }

                seat()->imp()->setActiveToplevel(this);
            }
        }

        if (m_current.decorationMode != pending->decorationMode)
            changesToNotify.add(DecorationModeChanged);
    }

    // If never assigned, use the surface + subsurfaces bounds
    if (!pending->windowGeometrySet)
        pending->windowGeometry = RXdgSurface::CalculateGeometryWithSubsurfaces(surface());

    if (m_current.windowGeometry != pending->windowGeometry)
        changesToNotify.add(WindowGeometryChanged);

    if (changesToNotify == 0)
        return;

    const auto prev { m_current };
    m_current = *pending;

    stateChanged(changesToNotify, prev);

    const CZBitset<CZWindowState> stateChanges { prev.windowState ^ m_current.windowState };

    if (stateChanges.has(CZWinActivated | CZWinMaximized | CZWinFullscreen))
    {
        for (auto *controller : foreignControllers())
        {
            controller->resource().updateState();
            controller->resource().done();
        }
    }

    if (changesToNotify.has(WindowGeometryChanged))
        m_resizeSession.handleGeometryChange();
}
