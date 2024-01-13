#include <protocols/WpPresentationTime/private/RWpPresentationFeedbackPrivate.h>
#include <protocols/Viewporter/private/RViewportPrivate.h>
#include <protocols/FractionalScale/private/RFractionalScalePrivate.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <protocols/Wayland/GCompositor.h>
#include <protocols/Wayland/GOutput.h>
#include <protocols/Wayland/RCallback.h>
#include <private/LBaseSurfaceRolePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LClientPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LPointerPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <private/LSubsurfaceRolePrivate.h>

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

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
    .set_buffer_transform   = &RSurface::RSurfacePrivate::set_buffer_transform,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
    .set_buffer_scale       = &RSurface::RSurfacePrivate::set_buffer_scale,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 4
    .damage_buffer          = &RSurface::RSurfacePrivate::damage_buffer,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    .offset                 = &RSurface::RSurfacePrivate::offset
#endif
};

RSurface::RSurface
(
    GCompositor *gCompositor,
    UInt32 id
)
    :LResource
    (
        gCompositor->client(),
        &wl_surface_interface,
        gCompositor->version(),
        id,
        &surface_implementation,
        &RSurface::RSurfacePrivate::resource_destroy
    ),
    LPRIVATE_INIT_UNIQUE(RSurface)
{
    // Create surface
    LSurface::Params params;
    params.surfaceResource = this;
    imp()->lSurface = compositor()->createSurfaceRequest(&params);

    // Append surface
    client()->imp()->surfaces.push_back(surface());
    surface()->imp()->clientLink = std::prev(client()->imp()->surfaces.end());
    compositor()->imp()->surfaces.push_back(surface());
    surface()->imp()->compositorLink = std::prev(compositor()->imp()->surfaces.end());
    compositor()->imp()->surfacesListChanged = true;
}

RSurface::~RSurface()
{
    LSurface *lSurface = this->surface();

    lSurface->imp()->setKeyboardGrabToParent();

    // Notify from client
    compositor()->destroySurfaceRequest(lSurface);

    // Unmap
    lSurface->imp()->setMapped(false);

    for (WpPresentationTime::RWpPresentationFeedback *wPf : lSurface->imp()->wpPresentationFeedbackResources)
        wPf->imp()->lSurface = nullptr;

    // Destroy pending frame callbacks
    while (!lSurface->imp()->frameCallbacks.empty())
    {
        Wayland::RCallback *rCallback = lSurface->imp()->frameCallbacks.front();
        rCallback->destroy();
    }

    // Clear keyboard focus
    if (seat()->keyboard()->focus() == lSurface)
        seat()->keyboard()->setFocus(nullptr);

    // Clear pointer focus
    if (seat()->pointer()->imp()->pointerFocusSurface == lSurface)
        seat()->pointer()->setFocus(nullptr);

    // Clear dragging surface
    if (seat()->pointer()->imp()->draggingSurface == lSurface)
        seat()->pointer()->imp()->draggingSurface = nullptr;

    if (seat()->pointer()->imp()->lastCursorRequest == lSurface->cursorRole())
        seat()->pointer()->imp()->lastCursorRequest = nullptr;

    // Clear active toplevel focus
    if (seat()->imp()->activeToplevel == lSurface->toplevel())
        seat()->imp()->activeToplevel = nullptr;

    // Clear moving toplevel
    if (seat()->pointer()->imp()->movingToplevel == lSurface->toplevel())
        seat()->pointer()->imp()->movingToplevel = nullptr;

    // Clear resizing toplevel
    if (seat()->pointer()->imp()->resizingToplevel == lSurface->toplevel())
        seat()->pointer()->imp()->resizingToplevel = nullptr;

    // Clear drag
    if (seat()->dndManager()->icon() && seat()->dndManager()->icon()->surface() == lSurface)
        seat()->dndManager()->imp()->icon = nullptr;

    if (lSurface->dndIcon())
    {
        LDNDIconRole *ldndIcon = lSurface->dndIcon();
        compositor()->destroyDNDIconRoleRequest(ldndIcon);
        delete ldndIcon;
    }

    if (lSurface->cursorRole())
    {
        LCursorRole *lCursor = lSurface->cursorRole();
        compositor()->destroyCursorRoleRequest(lCursor);
        delete lCursor;
    }

    if (imp()->rViewport)
        imp()->rViewport->imp()->rSurface = nullptr;

    if (imp()->rFractionalScale)
        imp()->rFractionalScale->imp()->rSurface = nullptr;

    while(!lSurface->children().empty())
        lSurface->imp()->removeChild(lSurface->imp()->children.back());

    while(!lSurface->imp()->pendingChildren.empty())
    {
        lSurface->imp()->pendingChildren.back()->imp()->pendingParent = nullptr;
        lSurface->imp()->pendingChildren.pop_back();
    }

    // Clear parent and pending parent
    lSurface->imp()->setParent(nullptr);

    if (lSurface->imp()->current.role)
        lSurface->imp()->current.role->imp()->surface = nullptr;

    if (lSurface->imp()->pending.role)
        lSurface->imp()->pending.role->imp()->surface = nullptr;

    // Remove surface from its client list
    lSurface->client()->imp()->surfaces.erase(lSurface->imp()->clientLink);

    // Remove the surface from the compositor list
    compositor()->imp()->surfaces.erase(lSurface->imp()->compositorLink);

    compositor()->imp()->surfacesListChanged = true;
    lSurface->imp()->destroyed = true;

    delete lSurface;
}

LSurface *RSurface::surface() const
{
    return imp()->lSurface;
}

FractionalScale::RFractionalScale *RSurface::fractionalScaleResource() const
{
    return imp()->rFractionalScale;
}

Viewporter::RViewport *RSurface::viewportResource() const
{
    return imp()->rViewport;
}

bool RSurface::enter(GOutput *gOutput)
{
    wl_surface_send_enter(resource(), gOutput->resource());
    return true;
}

bool RSurface::leave(GOutput *gOutput)
{
    wl_surface_send_leave(resource(), gOutput->resource());
    return true;
}

bool RSurface::preferredBufferScale(Int32 scale)
{
#if LOUVRE_WL_COMPOSITOR_VERSION >= 6
    if (version() >= 6)
    {
        wl_surface_send_preferred_buffer_scale(resource(), scale);
        return true;
    }
#endif
    L_UNUSED(scale);
    return false;
}

bool RSurface::preferredBufferTransform(UInt32 transform)
{
#if LOUVRE_WL_COMPOSITOR_VERSION >= 6
    if (version() >= 6)
    {
        wl_surface_send_preferred_buffer_transform(resource(), transform);
        return true;
    }
#endif
    L_UNUSED(transform);
    return false;
}
