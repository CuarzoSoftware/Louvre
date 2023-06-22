#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <protocols/Wayland/RRegion.h>
#include <protocols/Wayland/RCallback.h>
#include <private/LSurfacePrivate.h>
#include <LBaseSurfaceRole.h>
#include <LCompositor.h>
#include <LTime.h>
#include <LLog.h>

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

    lSurface->imp()->attached = true;

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

// The origin params indicates who requested the commit for this surface (itself or its parent surface)
void RSurface::RSurfacePrivate::apply_commit(LSurface *surface, CommitOrigin origin)
{
    // Check if the surface role wants to apply the commit
    if (surface->role() && !surface->role()->acceptCommitRequest(origin))
         return;

    surface->imp()->bufferSizeChanged = false;

    /**************************************
     *********** PENDING CHILDREN *********
     **************************************/
    surface->imp()->applyPendingChildren();

    /********************************************
     *********** NOTIFY PARENT COMMIT ***********
     ********************************************/

    for (LSurface *s : surface->children())
    {
        if (s->role())
            s->role()->handleParentCommit();
    }

    if (surface->imp()->attached)
    {
        surface->imp()->current.buffer = surface->imp()->pending.buffer;

        if (surface->imp()->current.buffer)
            surface->imp()->bufferReleased = false;

        surface->imp()->attached = false;
    }

    // Send done to already commited callbacks
    surface->requestNextFrame(false);

    // If new callbacks
    if (!surface->imp()->frameCallbacks.empty())
    {
        surface->requestedRepaint();
        for (RCallback *callback : surface->imp()->frameCallbacks)
            callback->commited = true;
    }

    /*****************************************
     *********** BUFFER TO TEXTURE ***********
     *****************************************/

    // Turn buffer into OpenGL texture and process damage
    if (surface->imp()->current.buffer)
    {
        if (!surface->imp()->bufferReleased)
        {
            // Returns false on wl_client destroy
            if (!surface->imp()->bufferToTexture())
            {
                LLog::error("[surface] Failed to convert buffer to OpenGL texture.");
                return;
            }
        }
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

    /*******************************************
     *********** NOTIFY COMMIT TO ROLE *********
     *******************************************/
    if (surface->role())
        surface->role()->handleSurfaceCommit(origin);
    else if (surface->imp()->pending.role)
    {
        surface->imp()->pending.role->handleSurfaceCommit(origin);
    }
    return;

    surface->imp()->bufferSizeChanged = false;
}

void RSurface::RSurfacePrivate::damage(wl_client *client, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    L_UNUSED(client);

    RSurface *lRSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = lRSurface->surface();

    // Ignore rects with invalid or insane sizes

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    if (height <= 0)
        return;

    lSurface->imp()->pendingDamagesS.push_back(LRect(x, y, width, height));
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

    // Ignore rects with invalid or insane sizes

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
    lSurface->imp()->pendingDamagesB.push_back(LRect(x, y, width, height));
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
