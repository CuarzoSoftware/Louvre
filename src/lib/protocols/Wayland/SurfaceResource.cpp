#include <protocols/Wayland/CompositorGlobal.h>
#include <protocols/Wayland/private/SurfaceResourcePrivate.h>
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
    .destroy                = &SurfaceResource::SurfaceResourcePrivate::destroy,
    .attach                 = &SurfaceResource::SurfaceResourcePrivate::attach,
    .damage                 = &SurfaceResource::SurfaceResourcePrivate::damage,
    .frame                  = &SurfaceResource::SurfaceResourcePrivate::frame,
    .set_opaque_region      = &SurfaceResource::SurfaceResourcePrivate::set_opaque_region,
    .set_input_region       = &SurfaceResource::SurfaceResourcePrivate::set_input_region,
    .commit                 = &SurfaceResource::SurfaceResourcePrivate::commit,

#if LOUVRE_COMPOSITOR_VERSION >= 2
    .set_buffer_transform   = &SurfaceResource::SurfaceResourcePrivate::set_buffer_transform,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 3
    .set_buffer_scale       = &SurfaceResource::SurfaceResourcePrivate::set_buffer_scale,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 4
    .damage_buffer          = &SurfaceResource::SurfaceResourcePrivate::damage_buffer,
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 5
    .offset                 = &SurfaceResource::SurfaceResourcePrivate::offset
#endif

};

SurfaceResource::SurfaceResource(CompositorGlobal *compositorGlobal,
                                 UInt32 id) :
    LResource(
        compositorGlobal->client(),
        &wl_surface_interface,
        compositorGlobal->version(),
        id,
        &surface_implementation,
        &SurfaceResource::SurfaceResourcePrivate::resource_destroy)
{
    m_imp = new SurfaceResourcePrivate();

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

SurfaceResource::~SurfaceResource()
{
    LSurface *surface = this->surface();

    // Unmap
    surface->imp()->setMapped(false);

    // Notify from client
    surface->compositor()->destroySurfaceRequest(surface);

    // Clear keyboard focus
    if(surface->seat()->keyboard()->focusSurface() == surface)
        surface->seat()->keyboard()->imp()->keyboardFocusSurface = nullptr;

    // Clear pointer focus
    if(surface->seat()->pointer()->imp()->pointerFocusSurface == surface)
        surface->seat()->pointer()->imp()->pointerFocusSurface = nullptr;

    // Clear dragging surface
    if(surface->seat()->pointer()->imp()->draggingSurface == surface)
        surface->seat()->pointer()->imp()->draggingSurface = nullptr;

    // Clear active toplevel focus
    if(surface->seat()->imp()->activeToplevel == surface->toplevel())
        surface->seat()->imp()->activeToplevel = nullptr;

    // Clear moving toplevel
    if(surface->seat()->pointer()->imp()->movingToplevel == surface->toplevel())
        surface->seat()->pointer()->imp()->movingToplevel = nullptr;

    // Clear resizing toplevel
    if(surface->seat()->pointer()->imp()->resizingToplevel == surface->toplevel())
        surface->seat()->pointer()->imp()->resizingToplevel = nullptr;

    // Clear drag
    if(surface->seat()->dndManager()->icon() && surface->seat()->dndManager()->icon()->surface() == surface)
        surface->seat()->dndManager()->imp()->icon = nullptr;

    if(surface->dndIcon())
    {
        LDNDIconRole *ldndIcon = surface->dndIcon();
        surface->compositor()->destroyDNDIconRoleRequest(ldndIcon);
        delete ldndIcon;
    }

    if(surface->cursor())
    {
        LCursorRole *lCursor = surface->cursor();
        surface->compositor()->destroyCursorRoleRequest(lCursor);
        delete lCursor;
    }


    while(!surface->children().empty())
    {
        surface->imp()->removeChild(surface->imp()->children.back());
    }

    while(!surface->imp()->pendingChildren.empty())
    {
        surface->imp()->pendingChildren.back()->imp()->pendingParent = nullptr;
        surface->imp()->pendingChildren.pop_back();
    }

    // Parent
    surface->imp()->setParent(nullptr);

    if(surface->imp()->pendingParent)
        surface->imp()->pendingParent->imp()->pendingChildren.remove(surface);

    if(surface->imp()->current.role)
        surface->imp()->current.role->baseImp()->surface = nullptr;

    if(surface->imp()->pending.role)
        surface->imp()->pending.role->baseImp()->surface = nullptr;

    if(surface->imp()->xdgSurfaceResource)
        wl_resource_set_user_data(surface->imp()->xdgSurfaceResource, nullptr);

    // Remove surface from its client list
    surface->client()->imp()->surfaces.erase(surface->imp()->clientLink);

    // Remove the surface from the compositor list
    surface->compositor()->imp()->surfaces.erase(surface->imp()->compositorLink);

    delete surface;

    delete m_imp;
}

LSurface *SurfaceResource::surface() const
{
    return imp()->surface;
}

SurfaceResource::SurfaceResourcePrivate *SurfaceResource::imp() const
{
    return m_imp;
}
