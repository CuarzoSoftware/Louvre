#include <private/LToplevelRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <protocols/XdgShell/xdg-shell.h>
#include <protocols/XdgDecoration/xdg-decoration-unstable-v1.h>

#include <LCompositor.h>
#include <LWayland.h>
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
    m_imp->toplevel = this;
    m_imp->appId = new char[1];
    m_imp->title = new char[1];
    m_imp->appId[0] = '\0';
    m_imp->title[0] = '\0';
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

    delete []m_imp->appId;
    delete []m_imp->title;
    delete m_imp;
}
#if LOUVRE_XDG_WM_BASE_VERSION >= XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION
UChar8 LToplevelRole::wmCapabilities() const
{
    return m_imp->wmCapabilities;
}

void LToplevelRole::setWmCapabilities(UChar8 capabilitiesFlags)
{
    m_imp->wmCapabilities = capabilitiesFlags;

    if (wl_resource_get_version(resource()) < XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION)
        return;


    wl_array dummy;
    wl_array_init(&dummy);
    UInt32 index = 0;

    if (m_imp->wmCapabilities & WmCapabilities::WmWindowMenu)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_WINDOW_MENU;
        index++;
    }
    if (m_imp->wmCapabilities & WmCapabilities::WmMaximize)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_MAXIMIZE;
        index++;
    }
    if (m_imp->wmCapabilities & WmCapabilities::WmFullscreen)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_FULLSCREEN;
        index++;
    }
    if (m_imp->wmCapabilities & WmCapabilities::WmMinimize)
    {
        wl_array_add(&dummy, sizeof(UInt32));
        UInt32 *s = (UInt32*)dummy.data;
        s[index] = XDG_TOPLEVEL_WM_CAPABILITIES_MINIMIZE;
        index++;
    }

    xdg_toplevel_send_wm_capabilities(resource(), &dummy);
    wl_array_release(&dummy);
    configureC(states());
}
#endif

void LToplevelRole::ping(UInt32 serial)
{
    xdg_wm_base_send_ping(surface()->client()->xdgWmBaseResource(),serial);
}


bool LToplevelRole::maximized() const
{
    return bool(m_imp->stateFlags & LToplevelRole::Maximized);
}

bool LToplevelRole::fullscreen() const
{
    return m_imp->stateFlags & LToplevelRole::Fullscreen;
}

bool LToplevelRole::activated() const
{
    return m_imp->stateFlags & LToplevelRole::Activated;
}

UInt32 LToplevelRole::states() const
{
    return m_imp->stateFlags;
}

void LToplevelRole::setDecorationMode(DecorationMode mode)
{
    if (decorationMode() == mode)
        return;

    m_imp->pendingDecorationMode = mode;

    if (!m_imp->xdgDecoration)
        return;

    m_imp->lastDecorationModeConfigureSerial = LWayland::nextSerial();
    zxdg_toplevel_decoration_v1_send_configure(m_imp->xdgDecoration, mode);
}

LToplevelRole::DecorationMode LToplevelRole::decorationMode() const
{
    return m_imp->decorationMode;
}

LToplevelRole::LToplevelRolePrivate *LToplevelRole::imp() const
{
    return m_imp;
}


void LToplevelRole::handleSurfaceCommit()
{

    // Commit inicial para asignar rol
    if (surface()->imp()->pending.role)
    {
        if (surface()->buffer())
        {
            wl_resource_post_error(resource(), XDG_SURFACE_ERROR_ALREADY_CONSTRUCTED, "Given wl_surface already has a buffer attached.");
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
        imp()->hasPendingWindowGeometry = false;
        imp()->currentMinSizeS = LSize();
        imp()->currentMinSizeC = LSize();
        imp()->pendingMinSizeS = LSize();
        imp()->currentMaxSizeS = LSize();
        imp()->currentMaxSizeC = LSize();
        imp()->pendingMaxSizeS = LSize();
        imp()->currentWindowGeometryS = LRect();
        imp()->currentWindowGeometryC = LRect();
        imp()->pendingWindowGeometryS = LRect();

        #if LOUVRE_XDG_WM_BASE_VERSION >= XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION
        imp()->boundsC = LSize();
        imp()->boundsS = LSize();
        #endif

        #if LOUVRE_XDG_WM_BASE_VERSION >= XDG_TOPLEVEL_WM_CAPABILITIES_SINCE_VERSION
        imp()->wmCapabilities = 0;
        #endif

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

    if (imp()->hasPendingWindowGeometry)
    {
        imp()->hasPendingWindowGeometry = false;
        imp()->currentWindowGeometryS = imp()->pendingWindowGeometryS;
        imp()->currentWindowGeometryC = imp()->pendingWindowGeometryS * compositor()->globalScale();
        geometryChanged();
    }
    // Si nunca ha asignado la geometría, usa el tamaño de la superficie
    else if (!imp()->windowGeometrySet && imp()->currentWindowGeometryC.bottomRight() != surface()->sizeC())
    {
        imp()->currentWindowGeometryS = LRect(0,surface()->sizeS());
        imp()->currentWindowGeometryC = LRect(0,surface()->sizeC());
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
                seat()->activeToplevel()->configureC(0, seat()->activeToplevel()->imp()->currentConf.flags & ~LToplevelRole::Activated);

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
    imp()->currentWindowGeometryC = imp()->currentWindowGeometryS * newScale;

    #if LOUVRE_XDG_WM_BASE_VERSION >= XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION
    imp()->boundsC = imp()->boundsS * newScale;
    #endif
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
    LToplevelRolePrivate::ToplevelConfiguration conf;

    conf.serial = LWayland::nextSerial();
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
    if (wl_resource_get_version(resource()) >= 2)
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

    m_imp->sentConfs.push_back(conf);
    xdg_toplevel_send_configure(resource(), conf.sizeS.w(), conf.sizeS.h(), &dummy);
    wl_array_release(&dummy);


    if (surface()->xdgRSurface())
        xdg_surface_send_configure(surface()->xdgRSurface(),conf.serial);

}

void LToplevelRole::close()
{
    xdg_toplevel_send_close(resource());
}

const LRect &LToplevelRole::windowGeometryS() const
{
    return imp()->currentWindowGeometryS;
}

const LRect &LToplevelRole::windowGeometryC() const
{
    return imp()->currentWindowGeometryC;
}

#if LOUVRE_XDG_WM_BASE_VERSION >= XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION
    void LToplevelRole::configureBoundsC(const LSize &bounds)
    {

        m_imp->boundsC = bounds;
        m_imp->boundsS = bounds/compositor()->globalScale();

        if (wl_resource_get_version(resource()) <= XDG_TOPLEVEL_CONFIGURE_BOUNDS_SINCE_VERSION)
            return;

        xdg_toplevel_send_configure_bounds(
                    resource(),
                    m_imp->boundsS.w() / compositor()->globalScale(),
                    m_imp->boundsS.h() / compositor()->globalScale());
        configureC(states());
    }

    const LSize &LToplevelRole::boundsS() const
    {
        return m_imp->boundsS;
    }

    const LSize &LToplevelRole::boundsC() const
    {
        return m_imp->boundsC;
    }
#endif

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
    strcpy(appId,newAppId);
    toplevel->appIdChanged();
}

void LToplevelRole::LToplevelRolePrivate::setTitle(const char *newTitle)
{
    delete []title;
    title = new char[strlen(newTitle)+1];
    strcpy(title,newTitle);
    toplevel->titleChanged();
}

