#include <protocols/XdgDecoration/RXdgToplevelDecoration.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <LToplevelResizeSession.h>
#include <LSubsurfaceRole.h>
#include <LCompositor.h>
#include <LOutput.h>
#include <LPoint.h>
#include <LCursor.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTime.h>
#include <string.h>
#include <ctype.h>

#undef None

using namespace Louvre;
using namespace Louvre::Protocols::XdgShell;

LToplevelRole::LToplevelRole(const void *params) noexcept :
    LBaseSurfaceRole(FactoryObjectType,
        ((LToplevelRole::Params*)params)->toplevel,
        ((LToplevelRole::Params*)params)->surface,
        LSurface::Role::Toplevel),
    LPRIVATE_INIT_UNIQUE(LToplevelRole)
{
    imp()->toplevel = this;
    imp()->moveSession.m_toplevel = this;
    imp()->resizeSession.m_toplevel = this;

    if (resource()->version() >= 2)
        imp()->supportedStates.add(TiledLeft | TiledTop | TiledRight | TiledBottom);

    if (resource()->version() >= 6)
        imp()->supportedStates.add(Suspended);
}

LToplevelRole::~LToplevelRole()
{
    // Required by pimpl
}

const LToplevelRole::Configuration *LToplevelRole::findConfiguration(UInt32 serial) const noexcept
{
    for (auto &conf : imp()->sentConfs)
        if (conf.serial == serial)
            return &conf;

    if (current().serial == serial)
        return &current();
    if (pending().serial == serial)
        return &current();

    return nullptr;
}

LBitset<LToplevelRole::State> LToplevelRole::supportedStates() const noexcept
{
    return imp()->supportedStates;
}

const LToplevelRole::Configuration &LToplevelRole::current() const noexcept
{
    return imp()->current;
}

const LToplevelRole::Configuration &LToplevelRole::pending() const noexcept
{
    return imp()->pending;
}

const LToplevelRole::Configuration &LToplevelRole::previous() const noexcept
{
    return imp()->previous;
}

void LToplevelRole::configureState(LBitset<State> flags) noexcept
{
    if (!imp()->stateFlags.check(LToplevelRolePrivate::HasConfigurationToSend))
    {
        imp()->pending.serial = LTime::nextSerial();
        imp()->stateFlags.add(LToplevelRolePrivate::HasConfigurationToSend);
    }

    imp()->pending.state = flags;
    compositor()->imp()->unlockPoll();
}

void LToplevelRole::configureSize(const LSize &size) noexcept
{
    if (!imp()->stateFlags.check(LToplevelRolePrivate::HasConfigurationToSend))
    {
        imp()->pending.serial = LTime::nextSerial();
        imp()->stateFlags.add(LToplevelRolePrivate::HasConfigurationToSend);
    }

    imp()->pending.size.setW(size.w() < 0 ? 0 : size.w());
    imp()->pending.size.setH(size.h() < 0 ? 0 : size.h());
    compositor()->imp()->unlockPoll();
}

void LToplevelRole::configureSize(Int32 width, Int32 height) noexcept
{
    if (!imp()->stateFlags.check(LToplevelRolePrivate::HasConfigurationToSend))
    {
        imp()->pending.serial = LTime::nextSerial();
        imp()->stateFlags.add(LToplevelRolePrivate::HasConfigurationToSend);
    }

    imp()->pending.size.setW(width < 0 ? 0 : width);
    imp()->pending.size.setH(height < 0 ? 0 : height);
    compositor()->imp()->unlockPoll();
}

void LToplevelRole::configureDecorationMode(DecorationMode mode) noexcept
{
    if (imp()->pending.decorationMode == mode)
        return;

    if (mode != ClientSide && mode != ServerSide)
        mode = ClientSide;

    if (!imp()->stateFlags.check(LToplevelRolePrivate::HasConfigurationToSend))
    {
        imp()->pending.serial = LTime::nextSerial();
        imp()->stateFlags.add(LToplevelRolePrivate::HasConfigurationToSend);
    }

    imp()->pending.decorationMode = mode;
}

LToplevelRole::DecorationMode LToplevelRole::decorationMode() const noexcept
{
    return imp()->current.decorationMode;
}

bool LToplevelRole::activated() const noexcept
{
    return imp()->current.state.check(Activated);
}

bool LToplevelRole::maximized() const noexcept
{
    return imp()->current.state.check(Maximized);
}

bool LToplevelRole::fullscreen() const noexcept
{
    return imp()->current.state.check(Fullscreen);
}

bool LToplevelRole::tiled() const noexcept
{
    return imp()->current.state.check(TiledLeft | TiledBottom | TiledTop | TiledRight);
}

bool LToplevelRole::resizing() const noexcept
{
    return imp()->current.state.check(Resizing);
}

LToplevelRole::DecorationMode LToplevelRole::preferredDecorationMode() const
{
    return imp()->preferredDecorationMode;
}

RXdgToplevel *LToplevelRole::xdgToplevelResource() const
{
    return static_cast<RXdgToplevel*>(resource());
}

RXdgSurface *LToplevelRole::xdgSurfaceResource() const
{
    return xdgToplevelResource()->xdgSurfaceRes();
}

LToplevelMoveSession &LToplevelRole::moveSession() const
{
    return imp()->moveSession;
}

LToplevelResizeSession &LToplevelRole::resizeSession() const
{
    return imp()->resizeSession;
}

void LToplevelRole::handleSurfaceCommit(LBaseSurfaceRole::CommitOrigin origin)
{
    L_UNUSED(origin);

    // Apply pending role
    if (surface()->imp()->pending.role)
    {
        if (imp()->stateFlags.check(LToplevelRolePrivate::HasPendingMaxSize))
        {
            imp()->stateFlags.remove(LToplevelRolePrivate::HasPendingMaxSize);
            imp()->currentMaxSize = imp()->pendingMaxSize;
        }

        if (imp()->stateFlags.check(LToplevelRolePrivate::HasPendingMinSize))
        {
            imp()->stateFlags.remove(LToplevelRolePrivate::HasPendingMinSize);
            imp()->currentMinSize = imp()->pendingMinSize;
        }

        if (xdgSurfaceResource()->m_hasPendingWindowGeometry)
        {
            xdgSurfaceResource()->m_hasPendingWindowGeometry = false;
            xdgSurfaceResource()->m_currentWindowGeometry = xdgSurfaceResource()->m_pendingWindowGeometry;
        }
        // Si nunca ha asignado la geometría, usa el tamaño de la superficie
        else if (!xdgSurfaceResource()->m_windowGeometrySet &&
                 xdgSurfaceResource()->m_currentWindowGeometry.size() != surface()->size())
        {
            xdgSurfaceResource()->m_currentWindowGeometry = LRect(0, surface()->size());
        }

        if (surface()->buffer())
        {
            wl_resource_post_error(resource()->resource(), XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
            return;
        }

        surface()->imp()->applyPendingRole();
        configureRequest();

        if (!imp()->stateFlags.check(LToplevelRolePrivate::HasConfigurationToSend))
            configureState(pending().state);

        // Fake set maximized or fullscreen request if called before the role was applied
        if (imp()->prevRoleRequest == LToplevelRole::Maximized)
            setMaximizedRequest();
        else if (imp()->prevRoleRequest == LToplevelRole::Fullscreen)
        {
            // Validate the output is still initialized
            if (imp()->prevRoleFullscreenRequestOutput)
            {
                for (LOutput *output : compositor()->outputs())
                {
                    if (output == imp()->prevRoleFullscreenRequestOutput)
                        goto skipUnsetOutput;
                }

                imp()->prevRoleFullscreenRequestOutput = nullptr;
            }

        skipUnsetOutput:
            setFullscreenRequest(imp()->prevRoleFullscreenRequestOutput);
        }

        imp()->prevRoleRequest = 0;
        imp()->prevRoleFullscreenRequestOutput = nullptr;

        return;
    }

    /* First apply all changes */

    LBitset<ConfigurationChanges> changes;

    if (imp()->stateFlags.check(LToplevelRolePrivate::HasPendingMaxSize))
    {
        changes.add(MaxSizeChanged);
        imp()->stateFlags.remove(LToplevelRolePrivate::HasPendingMaxSize);
        imp()->currentMaxSize = imp()->pendingMaxSize;
    }

    if (imp()->stateFlags.check(LToplevelRolePrivate::HasPendingMinSize))
    {
        changes.add(MinSizeChanged);
        imp()->stateFlags.remove(LToplevelRolePrivate::HasPendingMinSize);
        imp()->currentMinSize = imp()->pendingMinSize;
    }

    if (xdgSurfaceResource()->m_hasPendingWindowGeometry)
        xdgSurfaceResource()->m_currentWindowGeometry = xdgSurfaceResource()->m_pendingWindowGeometry;
    // If never assigned, use the surface + subsurfaces bounds
    else if (!xdgSurfaceResource()->m_windowGeometrySet)
    {
        const LRect newGeometry { xdgSurfaceResource()->calculateGeometryWithSubsurfaces() };

        if (xdgSurfaceResource()->m_currentWindowGeometry != newGeometry)
        {
            xdgSurfaceResource()->m_currentWindowGeometry = newGeometry;
            xdgSurfaceResource()->m_hasPendingWindowGeometry = true;
        }
    }

    /* Then notify all changes */

    if (xdgSurfaceResource()->m_hasPendingWindowGeometry)
    {
        changes.add(WindowGeometryChanged);
        xdgSurfaceResource()->m_hasPendingWindowGeometry = false;
    }

    imp()->applyPendingChanges(changes);

    // Request configure
    if (!surface()->mapped() && !surface()->buffer())
    {
        configureRequest();

        if (!imp()->stateFlags.check(LToplevelRolePrivate::HasConfigurationToSend))
            configureState(pending().state);
           
        return;
    }

    // Request unmap
    if (surface()->mapped() && !surface()->buffer())
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

        if (seat()->activeToplevel() == this)
            seat()->imp()->activeToplevel = nullptr;

        // Clean Up
        imp()->setAppId("");
        imp()->setTitle("");

        imp()->sentConfs.clear();
        imp()->stateFlags.remove(LToplevelRolePrivate::HasPendingMaxSize | LToplevelRolePrivate::HasPendingMinSize);
        imp()->currentMinSize = LSize();
        imp()->pendingMinSize = LSize();
        imp()->currentMaxSize = LSize();
        imp()->pendingMaxSize = LSize();

        xdgSurfaceResource()->m_hasPendingWindowGeometry = false;
        xdgSurfaceResource()->m_windowGeometrySet = false;
        xdgSurfaceResource()->m_pendingWindowGeometry = LRect();
        xdgSurfaceResource()->m_currentWindowGeometry = LRect();

        LBitset<ConfigurationChanges> changes { WindowGeometryChanged | StateChanged | MaxSizeChanged | MinSizeChanged };

        if (imp()->current.decorationMode != ClientSide)
        {
            imp()->current.decorationMode = ClientSide;
            changes.add(DecorationModeChanged);
        }

        imp()->current.size = LSize();
        imp()->current.state = NoState;
        imp()->current.serial = 0;
        imp()->pending = imp()->current;

        configurationChanged(changes);

        imp()->previous = imp()->current;
        return;
    }

    // Request map
    if (!surface()->mapped() && surface()->buffer())
        surface()->imp()->setMapped(true);
}

const std::string &LToplevelRole::appId() const
{
    return imp()->appId;
}

const std::string &LToplevelRole::title() const
{
    return imp()->title;
}

const LSize &LToplevelRole::maxSize() const
{
    return imp()->currentMaxSize;
}

bool LToplevelRole::sizeInRange(const LSize &size) const
{
    return (minSize().w() <= size.w() || minSize().w() == 0) &&
           (maxSize().w() >= size.w() || maxSize().w() == 0) &&
           (minSize().h() <= size.h() || minSize().h() == 0) &&
           (maxSize().h() >= size.h() || maxSize().h() == 0);

}

const LSize &LToplevelRole::minSize() const
{
    return imp()->currentMinSize;
}

void LToplevelRole::close() const
{
    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();
    res->close();
}

const LRect &LToplevelRole::windowGeometry() const
{
    return xdgSurfaceResource()->windowGeometry();
}

LSize LToplevelRole::calculateResizeSize(const LPoint &cursorPosDelta, const LSize &initialSize, ResizeEdge edge)
{
    LSize newSize = initialSize;
    switch(edge)
    {
        case LToplevelRole::NoEdge:
        {

        }break;
        case LToplevelRole::Bottom:
        {
            newSize.setH(initialSize.h() - cursorPosDelta.y());
        }break;
        case LToplevelRole::Right:
        {
            newSize.setW(initialSize.w() - cursorPosDelta.x());
        }break;
        case LToplevelRole::BottomRight:
        {
            newSize.setH(initialSize.h() - cursorPosDelta.y());
            newSize.setW(initialSize.w() - cursorPosDelta.x());
        }break;
        case LToplevelRole::Top:
        {
            newSize.setH(initialSize.h() + cursorPosDelta.y());
        }break;
        case LToplevelRole::Left:
        {
            newSize.setW(initialSize.w() + cursorPosDelta.x());
        }break;
        case LToplevelRole::TopLeft:
        {
            newSize.setH(initialSize.h() + cursorPosDelta.y());
            newSize.setW(initialSize.w() + cursorPosDelta.x());
        }break;
        case LToplevelRole::BottomLeft:
        {
            newSize.setH(initialSize.h() - cursorPosDelta.y());
            newSize.setW(initialSize.w() + cursorPosDelta.x());
        }break;
        case LToplevelRole::TopRight:
        {
            newSize.setH(initialSize.h() + cursorPosDelta.y());
            newSize.setW(initialSize.w() - cursorPosDelta.x());
        }break;
    }

        return newSize;
}

static char *trim(char *s)
{
    char *ptr;
    if (!s)
            return NULL;   // handle NULL string
    if (!*s)
            return s;      // handle empty string
    for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace(*ptr); --ptr);
    ptr[1] = '\0';
    return s;
}
void LToplevelRole::LToplevelRolePrivate::setAppId(const char *newAppId)
{
    char *text = trim((char*)newAppId);

    if (strcmp(appId.c_str(), text) == 0)
        return;

    appId = text;
    toplevel->appIdChanged();
}

void LToplevelRole::LToplevelRolePrivate::setTitle(const char *newTitle)
{
    char *text = trim((char*)newTitle);

    if (strcmp(title.c_str(), text) == 0)
        return;

    title = text;
    toplevel->titleChanged();
}
