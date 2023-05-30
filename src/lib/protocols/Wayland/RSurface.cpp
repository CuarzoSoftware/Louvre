#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <protocols/WpPresentationTime/private/RWpPresentationFeedbackPrivate.h>
#include <protocols/Wayland/GCompositor.h>
#include <protocols/XdgShell/xdg-shell.h>

#include <private/LClientPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LDNDIconRolePrivate.h>
#include <private/LCursorRolePrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LSubsurfaceRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LToplevelRolePrivate.h>

#include <LPopupRole.h>
#include <LCursor.h>
#include <LTime.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/eventfd.h>

using namespace Protocols::Wayland;

struct wl_surface_interface surface_implementation =
{
    .destroy                = &RSurface::RSurfacePrivate::destroy,
    .attach                 = &RSurface::RSurfacePrivate::attach,
    .damage                 = &RSurface::RSurfacePrivate::damage,
    .frame                  = &RSurface::RSurfacePrivate::frame,
    .set_opaque_region      = &RSurface::RSurfacePrivate::set_opaque_region,
    .set_input_region       = &RSurface::RSurfacePrivate::set_input_region,
    .commit                 = &RSurface::RSurfacePrivate::commit,

#if LOUVRE_COMPOSITOR_VERSION >= 2
    .set_buffer_transform   = &RSurface::RSurfacePrivate::set_buffer_transform,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 3
    .set_buffer_scale       = &RSurface::RSurfacePrivate::set_buffer_scale,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 4
    .damage_buffer          = &RSurface::RSurfacePrivate::damage_buffer,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 5
    .offset                 = &RSurface::RSurfacePrivate::offset
#endif
};

RSurface::RSurface
(
    GCompositor *compositorGlobal,
    UInt32 id
)
    :LResource
    (
        compositorGlobal->client(),
        &wl_surface_interface,
        compositorGlobal->version(),
        id,
        &surface_implementation,
        &RSurface::RSurfacePrivate::resource_destroy
    )
{
    m_imp = new RSurfacePrivate();

    // Create surface
    LSurface::Params params;
    params.surfaceResource = this;
    imp()->surface = compositor()->createSurfaceRequest(&params);

    // Append surface
    client()->imp()->surfaces.push_back(surface());
    surface()->imp()->clientLink = std::prev(client()->imp()->surfaces.end());
    compositor()->imp()->surfaces.push_back(surface());
    surface()->imp()->compositorLink = std::prev(compositor()->imp()->surfaces.end());
}

RSurface::~RSurface()
{
    LSurface *surface = this->surface();

    // Unmap
    surface->imp()->setMapped(false);

    // Notify from client
    compositor()->destroySurfaceRequest(surface);

    /* TODO
    // Safe Xdg shell removal
    if (surface->popup())
    {

    }
    */

    for (WpPresentationTime::RWpPresentationFeedback *wPf : surface->imp()->wpPresentationFeedbackResources)
        wPf->imp()->lSurface = nullptr;

    // Clear keyboard focus
    if (seat()->keyboard()->focusSurface() == surface)
        seat()->keyboard()->imp()->keyboardFocusSurface = nullptr;

    // Clear pointer focus
    if (seat()->pointer()->imp()->pointerFocusSurface == surface)
        seat()->pointer()->imp()->pointerFocusSurface = nullptr;

    // Clear dragging surface
    if (seat()->pointer()->imp()->draggingSurface == surface)
        seat()->pointer()->imp()->draggingSurface = nullptr;

    // Clear active toplevel focus
    if (seat()->imp()->activeToplevel == surface->toplevel())
        seat()->imp()->activeToplevel = nullptr;

    // Clear moving toplevel
    if (seat()->pointer()->imp()->movingToplevel == surface->toplevel())
        seat()->pointer()->imp()->movingToplevel = nullptr;

    // Clear resizing toplevel
    if (seat()->pointer()->imp()->resizingToplevel == surface->toplevel())
        seat()->pointer()->imp()->resizingToplevel = nullptr;

    // Clear drag
    if (seat()->dndManager()->icon() && seat()->dndManager()->icon()->surface() == surface)
        seat()->dndManager()->imp()->icon = nullptr;

    if (surface->dndIcon())
    {
        LDNDIconRole *ldndIcon = surface->dndIcon();
        compositor()->destroyDNDIconRoleRequest(ldndIcon);
        delete ldndIcon;
    }

    if (surface->cursorRole())
    {
        LCursorRole *lCursor = surface->cursorRole();
        compositor()->destroyCursorRoleRequest(lCursor);
        delete lCursor;
    }

    while(!surface->children().empty())
        surface->imp()->removeChild(surface->imp()->children.back());

    while(!surface->imp()->pendingChildren.empty())
    {
        surface->imp()->pendingChildren.back()->imp()->pendingParent = nullptr;
        surface->imp()->pendingChildren.pop_back();
    }

    // Parent
    surface->imp()->setParent(nullptr);

    if (surface->imp()->pendingParent)
        surface->imp()->pendingParent->imp()->pendingChildren.remove(surface);

    if (surface->imp()->current.role)
        surface->imp()->current.role->baseImp()->surface = nullptr;

    if (surface->imp()->pending.role)
        surface->imp()->pending.role->baseImp()->surface = nullptr;

    // Remove surface from its client list
    surface->client()->imp()->surfaces.erase(surface->imp()->clientLink);

    // Remove the surface from the compositor list
    compositor()->imp()->surfaces.erase(surface->imp()->compositorLink);

    delete surface;

    delete m_imp;
}

LSurface *RSurface::surface() const
{
    return imp()->surface;
}
