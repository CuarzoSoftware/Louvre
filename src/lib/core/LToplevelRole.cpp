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

#undef None

using namespace Louvre;

LToplevelRole::LToplevelRole(Louvre::LToplevelRole::Params *params) : LBaseSurfaceRole(params->toplevel, params->surface, LSurface::Role::Toplevel)
{
    m_imp = new LToplevelRolePrivate();
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

    // Notify
    compositor()->destroyToplevelRoleRequest(this);

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

UChar8 LToplevelRole::wmCapabilities() const
{
    return imp()->wmCapabilities;
}

void LToplevelRole::setWmCapabilities(UChar8 capabilitiesFlags)
{
    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();

    imp()->wmCapabilities = capabilitiesFlags;

    if (res->version() < 5)
        return;

    wl_array dummy;
    wl_array_init(&dummy);

#if LOUVRE_XDG_WM_BASE_VERSION >= 5
    UInt32 index = 0;
    if (imp()->wmCapabilities & WmCapabilities::WmWindowMenu)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU;
        index++;
    }
    if (imp()->wmCapabilities & WmCapabilities::WmMaximize)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE;
        index++;
    }
    if (imp()->wmCapabilities & WmCapabilities::WmFullscreen)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN;
        index++;
    }
    if (imp()->wmCapabilities & WmCapabilities::WmMinimize)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE;
        index++;
    }
#endif

    if (res->wm_capabilities(&dummy))
        configureC(states());

    wl_array_release(&dummy);
}

bool LToplevelRole::maximized() const
{
    return bool(imp()->stateFlags & LToplevelRole::Maximized);
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

    imp()->pendingDecorationMode = mode;

    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();

    if (!imp()->xdgDecoration || !res->rXdgSurface())
        return;

    imp()->xdgDecoration->configure(mode);
}

LToplevelRole::DecorationMode LToplevelRole::decorationMode() const
{
    return imp()->decorationMode;
}

RXdgToplevel *LToplevelRole::xdgToplevelResource() const
{
    return (RXdgToplevel*)resource();
}

RXdgSurface *LToplevelRole::xdgSurfaceResource() const
{
    return xdgToplevelResource()->rXdgSurface();
}

void LToplevelRole::handleSurfaceCommit(Protocols::Wayland::RSurface::CommitOrigin origin)
{
    // Commit inicial para asignar rol
    if (surface()->imp()->pending.role)
    {
        if (surface()->buffer())
        {
            wl_resource_post_error(resource()->resource(), XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
            return;
        }

        surface()->imp()->applyPendingRole();
        configureRequest();
        return;
    }

    // Solicita volver a mapear
    if (!surface()->mapped() && !surface()->buffer())
    {
        configureRequest();
        return;
    }

    // Solicita desmapear
    if (surface()->mapped() && !surface()->buffer())
    {

        surface()->imp()->setMapped(false);

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
        imp()->currentConf.sizeS = LSize();
        imp()->currentConf.flags = NoState;
        imp()->currentConf.serial = 0;
        imp()->sentConfs.clear();
        imp()->hasPendingMinSize = false;
        imp()->hasPendingMaxSize = false;
        imp()->currentMinSizeS = LSize();
        imp()->currentMinSizeC = LSize();
        imp()->pendingMinSizeS = LSize();
        imp()->currentMaxSizeS = LSize();
        imp()->currentMaxSizeC = LSize();
        imp()->pendingMaxSizeS = LSize();

        XdgShell::RXdgToplevel *rXdgToplevel = (XdgShell::RXdgToplevel*)resource();
        rXdgToplevel->rXdgSurface()->imp()->hasPendingWindowGeometry = false;
        rXdgToplevel->rXdgSurface()->imp()->windowGeometrySet = false;
        rXdgToplevel->rXdgSurface()->imp()->pendingWindowGeometryS = LRect();
        rXdgToplevel->rXdgSurface()->imp()->currentWindowGeometryS = LRect();
        rXdgToplevel->rXdgSurface()->imp()->currentWindowGeometryC = LRect();

        // Since 4
        imp()->boundsC = LSize();
        imp()->boundsS = LSize();

        // Since 5
        imp()->wmCapabilities = 0;

        return;
    }

    // Cambios double-buffered

    if (imp()->hasPendingMaxSize)
    {
        imp()->hasPendingMaxSize = false;
        imp()->currentMaxSizeS = imp()->pendingMaxSizeS;
        imp()->currentMaxSizeC = imp()->pendingMaxSizeS * compositor()->globalScale();
        maxSizeChanged();
    }

    if (imp()->hasPendingMinSize)
    {
        imp()->hasPendingMinSize = false;
        imp()->currentMinSizeS = imp()->pendingMinSizeS;
        imp()->currentMinSizeC = imp()->pendingMinSizeS * compositor()->globalScale();
        minSizeChanged();
    }

    if (xdgSurfaceResource()->imp()->hasPendingWindowGeometry)
    {
        xdgSurfaceResource()->imp()->hasPendingWindowGeometry = false;
        xdgSurfaceResource()->imp()->currentWindowGeometryS = xdgSurfaceResource()->imp()->pendingWindowGeometryS;
        xdgSurfaceResource()->imp()->currentWindowGeometryC = xdgSurfaceResource()->imp()->pendingWindowGeometryS * compositor()->globalScale();
        geometryChanged();
    }
    // Si nunca ha asignado la geometría, usa el tamaño de la superficie
    else if (!xdgSurfaceResource()->imp()->windowGeometrySet &&
             xdgSurfaceResource()->imp()->currentWindowGeometryC.size() != surface()->sizeC())
    {
        xdgSurfaceResource()->imp()->currentWindowGeometryS = LRect(0, surface()->sizeS());
        xdgSurfaceResource()->imp()->currentWindowGeometryC = LRect(0, surface()->sizeC());
        geometryChanged();
    }

    if (!imp()->currentConf.commited)
    {
        imp()->currentConf.commited = true;

        UInt32 prevState = imp()->stateFlags;
        imp()->stateFlags = imp()->currentConf.flags;

        if ((prevState & LToplevelRole::Maximized) != (imp()->currentConf.flags & LToplevelRole::Maximized))
            maximizedChanged();
        if ((prevState & LToplevelRole::Fullscreen) != (imp()->currentConf.flags & LToplevelRole::Fullscreen))
            fullscreenChanged();

        if (imp()->currentConf.flags & LToplevelRole::Activated)
        {
            if (seat()->activeToplevel() && seat()->activeToplevel() != this)
            {
                seat()->activeToplevel()->configureC(seat()->activeToplevel()->windowGeometryC().size(),
                                                         seat()->activeToplevel()->states() & ~LToplevelRole::Activated);
            }

            seat()->imp()->activeToplevel = this;
        }

        if ((prevState & LToplevelRole::Activated) != (imp()->currentConf.flags & LToplevelRole::Activated))
            activatedChanged();
    }

    // Si no estaba mapeada y añade buffer la mappeamos
    if (!surface()->mapped() && surface()->buffer())
        surface()->imp()->setMapped(true);
}

void LToplevelRole::handleParentMappingChange()
{
    // Si el padre deja de estar mapeado, pasamos a ser hijos de su padre o de nullptr
    if (!surface()->parent()->mapped())
        surface()->imp()->setParent(surface()->parent()->parent());

}

void LToplevelRole::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);
    imp()->currentMinSizeC = imp()->currentMinSizeS * newScale;
    imp()->currentMaxSizeC = imp()->currentMaxSizeS * newScale;
    xdgSurfaceResource()->imp()->currentWindowGeometryC = xdgSurfaceResource()->imp()->currentWindowGeometryS * newScale;
    imp()->boundsC = imp()->boundsS * newScale;
}

const char *LToplevelRole::appId() const
{
    return imp()->appId;
}

const char *LToplevelRole::title() const
{
    return imp()->title;
}

const LSize &LToplevelRole::maxSizeC() const
{
    return imp()->currentMaxSizeC;
}

const LSize &LToplevelRole::minSizeS() const
{
    return imp()->currentMinSizeS;
}

const LSize &LToplevelRole::maxSizeS() const
{
    return imp()->currentMaxSizeS;
}

const LSize &LToplevelRole::minSizeC() const
{
    return imp()->currentMinSizeC;
}

void LToplevelRole::configureC(Int32 width, Int32 height, UInt32 stateFlags)
{
    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();

    LToplevelRolePrivate::ToplevelConfiguration conf;

    conf.serial = LCompositor::nextSerial();
    conf.flags = stateFlags;
    conf.sizeS.setW(width);
    conf.sizeS.setH(height);
    conf.sizeS /= compositor()->globalScale();
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
    res->configure(conf.sizeS.w(), conf.sizeS.h(), &dummy);
    wl_array_release(&dummy);

    if (res->rXdgSurface())
    {
        imp()->lastDecorationModeConfigureSerial = conf.serial;
        res->rXdgSurface()->configure(conf.serial);
    }
}

void LToplevelRole::close() const
{
    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();
    res->close();
}

const LRect &LToplevelRole::windowGeometryS() const
{
    return xdgSurfaceResource()->imp()->currentWindowGeometryS;
}

const LRect &LToplevelRole::windowGeometryC() const
{
    return xdgSurfaceResource()->imp()->currentWindowGeometryC;
}

void LToplevelRole::configureBoundsC(const LSize &bounds)
{
    XdgShell::RXdgToplevel *res = (XdgShell::RXdgToplevel*)resource();

    imp()->boundsC = bounds;
    imp()->boundsS = bounds/compositor()->globalScale();

    if (res->configure_bounds(imp()->boundsS.w() / compositor()->globalScale(),
                              imp()->boundsS.h() / compositor()->globalScale()))
    {
        configureC(states());
    }
}

const LSize &LToplevelRole::boundsS() const
{
    return imp()->boundsS;
}

const LSize &LToplevelRole::boundsC() const
{
    return imp()->boundsC;
}

void LToplevelRole::configureC(const LSize &size, UInt32 stateFlags)
{
    configureC(size.w(), size.h(), stateFlags);
}

void LToplevelRole::configureC(UInt32 stateFlags)
{
    configureC(windowGeometryC().size(), stateFlags);
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

void LToplevelRole::LToplevelRolePrivate::setAppId(const char *newAppId)
{
    delete []appId;
    appId = new char[strlen(newAppId)+1];
    strcpy(appId, newAppId);
    toplevel->appIdChanged();
}

void LToplevelRole::LToplevelRolePrivate::setTitle(const char *newTitle)
{
    delete []title;
    title = new char[strlen(newTitle)+1];
    strcpy(title, newTitle);
    toplevel->titleChanged();
}
