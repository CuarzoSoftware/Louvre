#include "LLog.h"
#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <protocols/Wayland/RRegion.h>
#include <protocols/Wayland/RCallback.h>
#include <private/LSurfacePrivate.h>
#include <LBaseSurfaceRole.h>
#include <LCompositor.h>
#include <LTime.h>

void RSurface::RSurfacePrivate::resource_destroy(wl_resource *resource)
{
    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    delete rSurface;
}

void RSurface::RSurfacePrivate::handleOffset(LSurface *lSurface, Int32 x, Int32 y)
{
    if (lSurface->role())
        lSurface->role()->handleSurfaceOffset(x, y);
}

void RSurface::RSurfacePrivate::attach(wl_client *client, wl_resource *resource, wl_resource *buffer, Int32 x, Int32 y)
{
    L_UNUSED(client);
    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();

    lSurface->imp()->atached = true;

    if (lSurface->role())
        lSurface->role()->handleSurfaceBufferAttach(buffer, x, y);

    lSurface->imp()->pending.buffer = buffer;

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    if (rSurface->version() < 5)
        handleOffset(lSurface, x, y);
    else
        if (x != 0 || y != 0)
            wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_OFFSET, "Buffer offset is invalid. Check wl_surface::offset (v5).");
#else
    handleOffset(lSurface, x, y);
#endif
}

void RSurface::RSurfacePrivate::frame(wl_client *client, wl_resource *resource, UInt32 callback)
{
    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();
    new Wayland::RCallback(client, callback, &lSurface->imp()->frameCallbacks);
}

void RSurface::RSurfacePrivate::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void RSurface::RSurfacePrivate::commit(wl_client *, wl_resource *resource)
{
    RSurface *lRSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *surface = lRSurface->surface();
    apply_commit(surface);
}

void RSurface::RSurfacePrivate::apply_commit(LSurface *surface, CommitOrigin origin)
{
    /**************************************
     *********** PENDING CHILDREN *********
     **************************************/
    surface->imp()->applyPendingChildren();

    // Verifica si el rol desea aplicar el commit
    if (surface->role() && !surface->role()->acceptCommitRequest(origin))
        return;

    if (surface->imp()->atached)
    {
        surface->imp()->current.buffer = surface->imp()->pending.buffer;

        if (surface->imp()->current.buffer)
            surface->imp()->bufferReleased = false;

        surface->imp()->atached = false;
    }

    /*****************************************
     *********** BUFFER TO TEXTURE ***********
     *****************************************/

    // Convierte el buffer a una textura OpenGL o copia los daños
    if (surface->imp()->current.buffer)
    {
        if (!surface->imp()->bufferReleased)
            if (!surface->imp()->bufferToTexture())
                return;
    }

    /*******************************************
     *********** NOTIFY COMMIT TO ROLE *********
     *******************************************/
    if (surface->role())
        surface->role()->handleSurfaceCommit();
    else if (surface->imp()->pending.role)
    {
        surface->imp()->pending.role->handleSurfaceCommit();
    }

    /********************************************
     *********** NOTIFY PARENT COMMIT ***********
     ********************************************/

    for (LSurface *s : surface->children())
    {
        if (s->role())
            s->role()->handleParentCommit();
    }

    /************************************
     *********** INPUT REGION ***********
     ************************************/
    if (surface->receiveInput())
    {
        if (surface->imp()->inputRegionIsInfinite)
        {
            if (surface->imp()->bufferSizeChanged)
            {
                surface->imp()->currentInputRegionS.clear();
                surface->imp()->currentInputRegionS.addRect(LRect(0,surface->sizeS()));
                surface->imp()->currentInputRegionC = surface->imp()->currentInputRegionS;
                surface->imp()->currentInputRegionC.multiply(compositor()->globalScale());
            }
        }
        else if (surface->imp()->inputRegionChanged || surface->imp()->bufferSizeChanged)
        {
            surface->imp()->currentInputRegionS = surface->imp()->pendingInputRegionS;
            surface->imp()->currentInputRegionS.clip(LRect(0,surface->sizeS()));
            surface->imp()->currentInputRegionC = surface->imp()->currentInputRegionS;
            surface->imp()->currentInputRegionC.multiply(compositor()->globalScale());
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
    if (surface->imp()->opaqueRegionChanged || surface->imp()->bufferSizeChanged)
    {
        surface->imp()->currentOpaqueRegionS = surface->imp()->pendingOpaqueRegionS;
        surface->imp()->currentOpaqueRegionS.clip(LRect(0,surface->sizeS()));
        surface->imp()->currentOpaqueRegionC = surface->imp()->currentOpaqueRegionS;
        surface->imp()->currentOpaqueRegionC.multiply(compositor()->globalScale());
        surface->imp()->opaqueRegionChanged = false;
        surface->opaqueRegionChanged();

        /*****************************************
         ********** TRANSLUCENT REGION ***********
         *****************************************/
        surface->imp()->currentTranslucentRegionS = surface->imp()->currentOpaqueRegionS;
        surface->imp()->currentTranslucentRegionS.inverse(LRect(0,surface->sizeS()));
        surface->imp()->currentTranslucentRegionC = surface->imp()->currentTranslucentRegionS;
        surface->imp()->currentTranslucentRegionC.multiply(compositor()->globalScale());
    }

    surface->imp()->bufferSizeChanged = false;
    surface->commited();
}


void RSurface::RSurfacePrivate::damage(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RSurface *lRSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lRSurface->surface();

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    if (height <= 0)
        return;

    if (compositor()->globalScale() != 1)
    {
        x--;
        y--;

        width+=2;
        height+=2;

        int modX = x % compositor()->globalScale();
        int modY = y % compositor()->globalScale();

        x -= modX;
        y -= modY;

        width += width % compositor()->globalScale() + modX;
        height += height % compositor()->globalScale() + modY;
    }

    lSurface->imp()->pendingDamagesS.addRect(LRect(x, y, width, height));
    lSurface->imp()->damagesChanged = true;
}

void RSurface::RSurfacePrivate::set_opaque_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();

    if (region)
    {
        RRegion *rRegion = (RRegion*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingOpaqueRegionS = rRegion->region();
    }
    else
        lSurface->imp()->pendingOpaqueRegionS.clear();

    lSurface->imp()->opaqueRegionChanged = true;
}

void RSurface::RSurfacePrivate::set_input_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();

    if (region == NULL)
    {
        lSurface->imp()->pendingInputRegionS.clear();
        lSurface->imp()->inputRegionIsInfinite = true;
    }
    else
    {
        RRegion *lRRegion = (RRegion*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingInputRegionS = lRRegion->region();
        lSurface->imp()->inputRegionIsInfinite = false;
    }

    lSurface->imp()->inputRegionChanged = true;
}

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
void RSurface::RSurfacePrivate::set_buffer_transform(wl_client *client, wl_resource *resource, Int32 transform)
{
    // TODO
    L_UNUSED(client);
    L_UNUSED(resource);
    L_UNUSED(transform);
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
void RSurface::RSurfacePrivate::set_buffer_scale(wl_client *client, wl_resource *resource, Int32 scale)
{
    L_UNUSED(client);

    if (scale <= 0)
    {
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_SCALE, "Buffer scale must be >= 1.");
        return;
    }

    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();
    lSurface->imp()->pending.bufferScale = scale;
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 4
void RSurface::RSurfacePrivate::damage_buffer(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    if (height <= 0)
        return;

    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();

    if (compositor()->globalScale() != 1)
    {
        x -= x % compositor()->globalScale();
        y -= y % compositor()->globalScale();
        width += width % compositor()->globalScale() + x % compositor()->globalScale();
        height += height % compositor()->globalScale() + y % compositor()->globalScale();
    }

    lSurface->imp()->pendingDamagesB.addRect(LRect(x, y, width, height));
    lSurface->imp()->damagesChanged = true;
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
void RSurface::RSurfacePrivate::offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y)
{
    L_UNUSED(client);
    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();
    handleOffset(lSurface, x, y);
}
#endif
