#include <protocols/XdgDecoration/private/RXdgToplevelDecorationPrivate.h>
#include <protocols/XdgShell/private/RXdgSurfacePrivate.h>
#include <protocols/XdgShell/RXdgToplevel.h>
#include <private/LToplevelRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <protocols/XdgShell/xdg-shell.h>
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

LToplevelRole::LToplevelRole(Louvre::LToplevelRole::Params *params) : LBaseSurfaceRole(params->toplevel, params->surface, LSurface::Role::Toplevel)
{
    m_imp = new LToplevelRolePrivate();
    imp()->currentConf.commited = true;
    imp()->toplevel = this;
    imp()->appId = new char[1];
    imp()->title = new char[1];
    imp()->appId[0] = '\0';
    imp()->title[0] = '\0';
}

LToplevelRole::~LToplevelRole()
{
    if (surface())
        surface()->imp()->setMapped(false);

    // Remove focus
    if (seat()->pointer()->resizingToplevel() == this)
        seat()->pointer()->stopResizingToplevel();

    if (seat()->pointer()->movingToplevel() == this)
        seat()->pointer()->stopMovingToplevel();

    if (seat()->activeToplevel() == this)
        seat()->imp()->activeToplevel = nullptr;

    if (imp()->xdgDecoration)
        imp()->xdgDecoration->imp()->lToplevelRole = nullptr;

    delete []imp()->appId;
    delete []imp()->title;
    delete m_imp;
}

bool LToplevelRole::maximized() const
{
    return imp()->stateFlags & LToplevelRole::Maximized;
}

bool LToplevelRole::fullscreen() const
{
    return imp()->stateFlags & LToplevelRole::Fullscreen;
}

bool LToplevelRole::activated() const
{
    return imp()->stateFlags & LToplevelRole::Activated;
}

UInt32 LToplevelRole::states() const
{
    return imp()->stateFlags;
}

void LToplevelRole::setDecorationMode(DecorationMode mode)
{
    if (decorationMode() == mode)
        return;

    if (mode != DecorationMode::ClientSide && mode != DecorationMode::ServerSide)
        return;

    imp()->pendingDecorationMode = mode;
}

LToplevelRole::DecorationMode LToplevelRole::decorationMode() const
{
    return imp()->decorationMode;
}

LToplevelRole::DecorationMode LToplevelRole::preferredDecorationMode() const
{
    return imp()->preferredDecorationMode;
}

RXdgToplevel *LToplevelRole::xdgToplevelResource() const
{
    return (RXdgToplevel*)resource();
}

RXdgSurface *LToplevelRole::xdgSurfaceResource() const
{
    return xdgToplevelResource()->xdgSurfaceResource();
}

void LToplevelRole::handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin)
{
    L_UNUSED(origin);

    // Apply pending role
    if (surface()->imp()->pending.role)
    {
        if (imp()->hasPendingMaxSize)
        {
            imp()->hasPendingMaxSize = false;
            imp()->currentMaxSize = imp()->pendingMaxSize;
        }

        if (imp()->hasPendingMinSize)
        {
            imp()->hasPendingMinSize = false;
            imp()->currentMinSize = imp()->pendingMinSize;
        }

        if (xdgSurfaceResource()->imp()->hasPendingWindowGeometry)
        {
            xdgSurfaceResource()->imp()->hasPendingWindowGeometry = false;
            xdgSurfaceResource()->imp()->currentWindowGeometry = xdgSurfaceResource()->imp()->pendingWindowGeometry;
        }
        // Si nunca ha asignado la geometría, usa el tamaño de la superficie
        else if (!xdgSurfaceResource()->imp()->windowGeometrySet &&
                 xdgSurfaceResource()->imp()->currentWindowGeometry.size() != surface()->size())
        {
            xdgSurfaceResource()->imp()->currentWindowGeometry = LRect(0, surface()->size());
        }

        if (surface()->buffer())
        {
            wl_resource_post_error(resource()->resource(), XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
            return;
        }

        surface()->imp()->applyPendingRole();
        configureRequest();

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

    if (imp()->hasPendingMaxSize)
        imp()->currentMaxSize = imp()->pendingMaxSize;

    if (imp()->hasPendingMinSize)
        imp()->currentMinSize = imp()->pendingMinSize;

    if (xdgSurfaceResource()->imp()->hasPendingWindowGeometry)
        xdgSurfaceResource()->imp()->currentWindowGeometry = xdgSurfaceResource()->imp()->pendingWindowGeometry;
    // If never assigned, use the surface size
    else if (!xdgSurfaceResource()->imp()->windowGeometrySet &&
             xdgSurfaceResource()->imp()->currentWindowGeometry.size() != surface()->size())
    {
        xdgSurfaceResource()->imp()->hasPendingWindowGeometry = true;
        xdgSurfaceResource()->imp()->currentWindowGeometry = LRect(0, surface()->size());
    }

    /* Then notify all changes */

    imp()->applyPendingChanges();

    if (imp()->hasPendingMinSize)
    {
        imp()->hasPendingMinSize = false;
        minSizeChanged();
    }

    if (imp()->hasPendingMaxSize)
    {
        imp()->hasPendingMaxSize = false;
        maxSizeChanged();
    }

    if (xdgSurfaceResource()->imp()->hasPendingWindowGeometry)
    {
        xdgSurfaceResource()->imp()->hasPendingWindowGeometry = false;
        geometryChanged();
    }

    // Request configure
    if (!surface()->mapped() && !surface()->buffer())
    {
        configureRequest();
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

        if (seat()->pointer()->movingToplevel() == this)
            seat()->pointer()->stopMovingToplevel();

        if (seat()->pointer()->resizingToplevel() == this)
            seat()->pointer()->stopResizingToplevel();

        if (seat()->activeToplevel() == this)
            seat()->imp()->activeToplevel = nullptr;

        // Clean Up
        imp()->setAppId("");
        imp()->setTitle("");
        imp()->stateFlags = NoState;
        imp()->currentConf.commited = false;
        imp()->currentConf.size = LSize();
        imp()->currentConf.flags = NoState;
        imp()->currentConf.serial = 0;
        imp()->sentConfs.clear();
        imp()->hasPendingMinSize = false;
        imp()->hasPendingMaxSize = false;
        imp()->currentMinSize = LSize();
        imp()->pendingMinSize = LSize();
        imp()->currentMaxSize = LSize();
        imp()->pendingMaxSize = LSize();

        XdgShell::RXdgToplevel *rXdgToplevel = (XdgShell::RXdgToplevel*)resource();
        rXdgToplevel->xdgSurfaceResource()->imp()->hasPendingWindowGeometry = false;
        rXdgToplevel->xdgSurfaceResource()->imp()->windowGeometrySet = false;
        rXdgToplevel->xdgSurfaceResource()->imp()->pendingWindowGeometry = LRect();
        rXdgToplevel->xdgSurfaceResource()->imp()->currentWindowGeometry = LRect();
        return;
    }

    // Request map
    if (!surface()->mapped() && surface()->buffer())
    {
        surface()->imp()->setMapped(true);
        return;
    }
}

const char *LToplevelRole::appId() const
{
    return imp()->appId;
}

const char *LToplevelRole::title() const
{
    return imp()->title;
}

const LSize &LToplevelRole::maxSize() const
{
    return imp()->currentMaxSize;
}

bool LToplevelRole::resizing() const
{
    return imp()->stateFlags & LToplevelRole::Resizing;
}

const LSize &LToplevelRole::minSize() const
{
    return imp()->currentMinSize;
}

void LToplevelRole::configure(Int32 width, Int32 height, UInt32 stateFlags)
{
    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();

    LToplevelRolePrivate::ToplevelConfiguration conf;

    surface()->requestNextFrame(false);

    if (width < 0)
        width = 0;

    if (height < 0)
        height = 0;

    conf.serial = LCompositor::nextSerial();
    conf.flags = stateFlags;
    conf.size.setW(width);
    conf.size.setH(height);
    conf.commited = false;

    wl_array dummy;
    wl_array_init(&dummy);
    UInt32 index = 0;

    if (conf.flags & LToplevelRole::Activated)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_ACTIVATED;
        index++;
    }
    if (conf.flags & LToplevelRole::Fullscreen)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_FULLSCREEN;
        index++;
    }
    if (conf.flags & LToplevelRole::Maximized)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_MAXIMIZED;
        index++;
    }
    if (conf.flags & LToplevelRole::Resizing)
    {
        wl_array_add(&dummy, sizeof(xdg_toplevel_state));
        xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
        s[index] = XDG_TOPLEVEL_STATE_RESIZING;
        index++;
    }

#if LOUVRE_XDG_WM_BASE_VERSION >= 2
    if (resource()->version() >= 2)
    {
        if (conf.flags & LToplevelRole::TiledBottom)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_BOTTOM;
            index++;
        }
        if (conf.flags & LToplevelRole::TiledLeft)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_LEFT;
            index++;
        }
        if (conf.flags & LToplevelRole::TiledRight)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_RIGHT;
            index++;
        }
        if (conf.flags & LToplevelRole::TiledTop)
        {
            wl_array_add(&dummy, sizeof(xdg_toplevel_state));
            xdg_toplevel_state *s = (xdg_toplevel_state*)dummy.data;
            s[index] = XDG_TOPLEVEL_STATE_TILED_TOP;
            index++;
        }
    }
#endif

    imp()->sentConfs.push_back(conf);
    res->configure(conf.size.w(), conf.size.h(), &dummy);
    wl_array_release(&dummy);

    if (res->xdgSurfaceResource())
    {
        if (imp()->pendingDecorationMode != 0 && imp()->xdgDecoration)
        {
            imp()->xdgDecoration->configure(imp()->pendingDecorationMode);
            imp()->lastDecorationModeConfigureSerial = conf.serial;
        }

        res->xdgSurfaceResource()->configure(conf.serial);
    }
}

void LToplevelRole::close() const
{
    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();
    res->close();
}

const LRect &LToplevelRole::windowGeometry() const
{
    return xdgSurfaceResource()->imp()->currentWindowGeometry;
}

void LToplevelRole::configure(const LSize &size, UInt32 stateFlags)
{
    configure(size.w(), size.h(), stateFlags);
}

void LToplevelRole::configure(UInt32 stateFlags)
{
    if (imp()->sentConfs.empty())
        configure(windowGeometry().size(), stateFlags);
    else
        configure(imp()->sentConfs.back().size, stateFlags);
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

void LToplevelRole::updateResizingPos()
{
    LSize s = imp()->resizingInitWindowSize;
    LPoint p = imp()->resizingInitPos;
    LToplevelRole::ResizeEdge edge =  imp()->resizingEdge;

    if (edge ==  LToplevelRole::Top || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::TopRight)
        surface()->setY(p.y() + (s.h() - windowGeometry().h()));

    if (edge ==  LToplevelRole::Left || edge ==  LToplevelRole::TopLeft || edge ==  LToplevelRole::BottomLeft)
        surface()->setX(p.x() + (s.w() - windowGeometry().w()));
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

    if (strcmp(appId, text) == 0)
        return;

    delete []appId;
    appId = new char[strlen(text)+1];
    strcpy(appId, text);
    toplevel->appIdChanged();
}

void LToplevelRole::LToplevelRolePrivate::setTitle(const char *newTitle)
{
    char *text = trim((char*)newTitle);

    if (strcmp(title, text) == 0)
        return;

    delete []title;
    title = new char[strlen(text)+1];
    strcpy(title, text);
    toplevel->titleChanged();
}

void LToplevelRole::LToplevelRolePrivate::applyPendingChanges()
{
    if (!currentConf.commited)
    {
        currentConf.commited = true;

        UInt32 prevState = stateFlags;
        stateFlags = currentConf.flags;

        if ((prevState & LToplevelRole::Maximized) != (stateFlags & LToplevelRole::Maximized))
            toplevel->maximizedChanged();

        if ((prevState & LToplevelRole::Fullscreen) != (stateFlags & LToplevelRole::Fullscreen))
            toplevel->fullscreenChanged();

        if (currentConf.flags & LToplevelRole::Activated)
        {
            if (seat()->activeToplevel() && seat()->activeToplevel() != toplevel)
                seat()->activeToplevel()->configure(seat()->activeToplevel()->states() & ~LToplevelRole::Activated);

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
