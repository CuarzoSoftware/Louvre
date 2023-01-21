#include <globals/Wayland/Surface.h>
#include <globals/XdgShell/xdg-shell.h>

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

using namespace Louvre;

void Globals::Surface::resource_destroy(wl_resource *resource)
{
    // Get surface
    LSurface *surface = (LSurface*)wl_resource_get_user_data(resource);

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
    surface->client()->imp()->surfaces.remove(surface);

    // Remove the surface from the compositor list
    surface->compositor()->imp()->surfaces.remove(surface);

    delete surface;
}

void Globals::Surface::handleOffset(LSurface *lSurface, Int32 x, Int32 y)
{
    if(lSurface->role())
        lSurface->role()->handleSurfaceOffset(x, y);
}

// SURFACE
void Globals::Surface::attach(wl_client *, wl_resource *resource, wl_resource *buffer, Int32 x, Int32 y)
{
    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if(lSurface->role())
        lSurface->role()->handleSurfaceBufferAttach(buffer, x, y);

    lSurface->imp()->pending.buffer = buffer;

#if LOUVRE_COMPOSITOR_VERSION >= 5
    if(wl_resource_get_version(resource) < 5)
        handleOffset(lSurface, x, y);
    else
    {
        if(x != 0 || y != 0)
            wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_OFFSET, "Buffer offset is invalid. Check wl_surface::offset (v5).");
    }
#else
    handleOffset(lSurface, x, y);
#endif

}

void Globals::Surface::frame(wl_client *client, wl_resource *resource, UInt32 callback)
{
    /* Client waits for this frame to be marked as done before creating next frame*/

    LSurface *surface = (LSurface*)wl_resource_get_user_data(resource);

    if(surface->imp()->frameCallback)
        wl_resource_destroy(surface->imp()->frameCallback);

    surface->imp()->frameCallback = wl_resource_create(client, &wl_callback_interface, 1, callback);
}

void Globals::Surface::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void Globals::Surface::commit(wl_client *, wl_resource *resource)
{
    LSurface *surface = (LSurface*)wl_resource_get_user_data(resource);
    apply_commit(surface);
}

void Globals::Surface::apply_commit(LSurface *surface, CommitOrigin origin)
{
    /**************************************
     *********** PENDING CHILDREN *********
     **************************************/
    surface->imp()->applyPendingChildren();

    /********************************************
     *********** NOTIFY PARENT COMMIT ***********
     ********************************************/

    for(LSurface *s : surface->children())
    {
        if(s->role())
            s->role()->handleParentCommit();
    }

    // Verifica si el rol desea aplicar el commit
    if(surface->role() && !surface->role()->acceptCommitRequest(origin))
        return;

    // Asigna el buffer pendiente al actual
    surface->imp()->current.buffer = surface->imp()->pending.buffer;

    /*****************************************
     *********** BUFFER TO TEXTURE ***********
     *****************************************/

    // Convierte el buffer a una textura OpenGL o copia los daños
    if(surface->imp()->current.buffer)
    {
        if(!surface->imp()->bufferToTexture())
            return;
    }


    /*******************************************
     *********** NOTIFY COMMIT TO ROLE *********
     *******************************************/
    if(surface->role())
        surface->role()->handleSurfaceCommit();
    else if(surface->imp()->pending.role)
    {
        surface->imp()->pending.role->handleSurfaceCommit();
    }


    /************************************
     *********** INPUT REGION ***********
     ************************************/
    if(surface->receiveInput())
    {
        if(surface->imp()->inputRegionIsInfinite)
        {
            if(surface->imp()->bufferSizeChanged)
            {
                surface->imp()->currentInputRegionS.clear();
                surface->imp()->currentInputRegionS.addRect(LRect(0,surface->sizeS()));
                surface->imp()->currentInputRegionC = surface->imp()->currentInputRegionS;
                surface->imp()->currentInputRegionC.multiply(surface->compositor()->globalScale());
            }
        }
        else if(surface->imp()->inputRegionChanged || surface->imp()->bufferSizeChanged)
        {
            surface->imp()->currentInputRegionS = surface->imp()->pendingInputRegionS;
            surface->imp()->currentInputRegionS.clip(LRect(0,surface->sizeS()));
            surface->imp()->currentInputRegionC = surface->imp()->currentInputRegionS;
            surface->imp()->currentInputRegionC.multiply(surface->compositor()->globalScale());
            surface->inputRegionChanged();
            surface->imp()->inputRegionChanged = false;
        }
    }
    else
    {
        /******************************************
         *********** CLEAR INPUT REGION ***********
         ******************************************/
        surface->imp()->currentInputRegionS.clear();
        surface->imp()->currentInputRegionC.clear();
    }

    /************************************
     ********** OPAQUE REGION ***********
     ************************************/
    if(surface->imp()->opaqueRegionChanged || surface->imp()->bufferSizeChanged)
    {
        surface->imp()->currentOpaqueRegionS = surface->imp()->pendingOpaqueRegionS;
        surface->imp()->currentOpaqueRegionS.clip(LRect(0,surface->sizeS()));
        surface->imp()->currentOpaqueRegionC = surface->imp()->currentOpaqueRegionS;
        surface->imp()->currentOpaqueRegionC.multiply(surface->compositor()->globalScale());
        surface->imp()->opaqueRegionChanged = false;
        surface->opaqueRegionChanged();

        /*****************************************
         ********** TRANSLUCENT REGION ***********
         *****************************************/
        surface->imp()->currentTranslucentRegionS = surface->imp()->currentOpaqueRegionS;
        surface->imp()->currentTranslucentRegionS.inverse(LRect(0,surface->sizeS()));
        surface->imp()->currentTranslucentRegionC = surface->imp()->currentTranslucentRegionS;
        surface->imp()->currentTranslucentRegionC.multiply(surface->compositor()->globalScale());

    }


    surface->imp()->bufferSizeChanged = false;
}


void Globals::Surface::damage(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if(width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;

    if(height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;


    if(lSurface->compositor()->globalScale() != 1)
    {
        x--;
        y--;

        width+=2;
        height+=2;

        int modX = x % lSurface->compositor()->globalScale();
        int modY = y % lSurface->compositor()->globalScale();

        x -= modX;
        y -= modY;

        width += width % lSurface->compositor()->globalScale() + modX;
        height += height % lSurface->compositor()->globalScale() + modY;
    }


    lSurface->imp()->pendingDamagesS.addRect(LRect(x, y, width, height));
    lSurface->imp()->damagesChanged = true;
}

void Globals::Surface::set_opaque_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if(region)
    {
        LRegion *lRegion = (LRegion*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingOpaqueRegionS = *lRegion;
    }
    else
        lSurface->imp()->pendingOpaqueRegionS.clear();

    lSurface->imp()->opaqueRegionChanged = true;
}

void Globals::Surface::set_input_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if(region == NULL)
    {
        lSurface->imp()->pendingInputRegionS.clear();
        lSurface->imp()->inputRegionIsInfinite = true;
    }
    else
    {
        LRegion *lRegion = (LRegion*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingInputRegionS = *lRegion;
        lSurface->imp()->inputRegionIsInfinite = false;
    }

    lSurface->imp()->inputRegionChanged = true;
}

#if LOUVRE_COMPOSITOR_VERSION >= 2
void Globals::Surface::set_buffer_transform(wl_client *client, wl_resource *resource, Int32 transform)
{
    // TODO
    L_UNUSED(client);
    L_UNUSED(resource);
    L_UNUSED(transform);
}
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 3
void Globals::Surface::set_buffer_scale(wl_client *client, wl_resource *resource, Int32 scale)
{
    L_UNUSED(client);

    if(scale <= 0)
    {
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_SCALE, "Buffer scale must be >= 1.");
        return;
    }

    LSurface *surface = (LSurface*)wl_resource_get_user_data(resource);
    surface->imp()->pending.bufferScale = scale;
}
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 4
void Globals::Surface::damage_buffer(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if(width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;

    if(height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);

    if(lSurface->compositor()->globalScale() != 1)
    {
        x-=x%lSurface->compositor()->globalScale();
        y-=y%lSurface->compositor()->globalScale();
        width+=width%lSurface->compositor()->globalScale() + x%lSurface->compositor()->globalScale();
        height+=height%lSurface->compositor()->globalScale() + y%lSurface->compositor()->globalScale();
    }

    lSurface->imp()->pendingDamagesB.addRect(LRect(x, y, width, height));
    lSurface->imp()->damagesChanged = true;
}
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 5
void Globals::Surface::offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y)
{
    L_UNUSED(client);

    LSurface *lSurface = (LSurface*)wl_resource_get_user_data(resource);
    handleOffset(lSurface, x, y);
}
#endif


