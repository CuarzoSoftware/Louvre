#include "protocols/Wayland/RegionResource.h"
#include <LBaseSurfaceRole.h>
#include <protocols/Wayland/private/SurfaceResourcePrivate.h>
#include <private/LSurfacePrivate.h>
#include <LTime.h>
#include <LCompositor.h>

void SurfaceResource::SurfaceResourcePrivate::resource_destroy(wl_resource *resource)
{
    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    delete lSurfaceResource;
}

void SurfaceResource::SurfaceResourcePrivate::handleOffset(LSurface *lSurface, Int32 x, Int32 y)
{
    if(lSurface->role())
        lSurface->role()->handleSurfaceOffset(x, y);
}

// SURFACE
void SurfaceResource::SurfaceResourcePrivate::attach(wl_client *, wl_resource *resource, wl_resource *buffer, Int32 x, Int32 y)
{
    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();

    if(lSurface->role())
        lSurface->role()->handleSurfaceBufferAttach(buffer, x, y);

    lSurface->imp()->pending.buffer = buffer;

    if(buffer)
        lSurface->imp()->bufferReleased = false;

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

void SurfaceResource::SurfaceResourcePrivate::frame(wl_client *client, wl_resource *resource, UInt32 callback)
{
    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();

    if(lSurface->imp()->frameCallback)
    {
        wl_callback_send_done(lSurface->imp()->frameCallback, LTime::ms());
        wl_resource_destroy(lSurface->imp()->frameCallback);
    }
    lSurface->imp()->frameCallback = wl_resource_create(client, &wl_callback_interface, 1, callback);
}

void SurfaceResource::SurfaceResourcePrivate::destroy(wl_client *, wl_resource *resource)
{
    wl_resource_destroy(resource);
}

void SurfaceResource::SurfaceResourcePrivate::commit(wl_client *, wl_resource *resource)
{
    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *surface = lSurfaceResource->surface();
    apply_commit(surface);
}

void SurfaceResource::SurfaceResourcePrivate::apply_commit(LSurface *surface, CommitOrigin origin)
{
    /**************************************
     *********** PENDING CHILDREN *********
     **************************************/
    surface->imp()->applyPendingChildren();

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
        if(!surface->imp()->bufferReleased)
        {
            if(!surface->imp()->bufferToTexture())
                return;
        }
    }
    else
    {
        if(surface->imp()->frameCallback)
            surface->requestNextFrame();
    }

    surface->repaintOutputs();
    /*******************************************
     *********** NOTIFY COMMIT TO ROLE *********
     *******************************************/
    if(surface->role())
        surface->role()->handleSurfaceCommit();
    else if(surface->imp()->pending.role)
    {
        surface->imp()->pending.role->handleSurfaceCommit();
    }

    /********************************************
     *********** NOTIFY PARENT COMMIT ***********
     ********************************************/

    for(LSurface *s : surface->children())
    {
        if(s->role())
            s->role()->handleParentCommit();
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


void SurfaceResource::SurfaceResourcePrivate::damage(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();

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

void SurfaceResource::SurfaceResourcePrivate::set_opaque_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();

    if(region)
    {
        RegionResource *lRegionResource = (RegionResource*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingOpaqueRegionS = lRegionResource->region();
    }
    else
        lSurface->imp()->pendingOpaqueRegionS.clear();

    lSurface->imp()->opaqueRegionChanged = true;
}

void SurfaceResource::SurfaceResourcePrivate::set_input_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();

    if(region == NULL)
    {
        lSurface->imp()->pendingInputRegionS.clear();
        lSurface->imp()->inputRegionIsInfinite = true;
    }
    else
    {
        RegionResource *lRegionResource = (RegionResource*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingInputRegionS = lRegionResource->region();
        lSurface->imp()->inputRegionIsInfinite = false;
    }

    lSurface->imp()->inputRegionChanged = true;
}

#if LOUVRE_COMPOSITOR_VERSION >= 2
void SurfaceResource::SurfaceResourcePrivate::set_buffer_transform(wl_client *client, wl_resource *resource, Int32 transform)
{
    // TODO
    L_UNUSED(client);
    L_UNUSED(resource);
    L_UNUSED(transform);
}
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 3
void SurfaceResource::SurfaceResourcePrivate::set_buffer_scale(wl_client *client, wl_resource *resource, Int32 scale)
{
    L_UNUSED(client);

    if(scale <= 0)
    {
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_SCALE, "Buffer scale must be >= 1.");
        return;
    }

    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();
    lSurface->imp()->pending.bufferScale = scale;
}
#endif

#if LOUVRE_COMPOSITOR_VERSION >= 4
void SurfaceResource::SurfaceResourcePrivate::damage_buffer(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    if(width > MAX_SURFACE_SIZE)
        width = MAX_SURFACE_SIZE;

    if(height > MAX_SURFACE_SIZE)
        height = MAX_SURFACE_SIZE;

    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();

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
void SurfaceResource::SurfaceResourcePrivate::offset(wl_client *client, wl_resource *resource, Int32 x, Int32 y)
{
    L_UNUSED(client);

    SurfaceResource *lSurfaceResource = (SurfaceResource*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lSurfaceResource->surface();
    handleOffset(lSurface, x, y);
}
#endif
