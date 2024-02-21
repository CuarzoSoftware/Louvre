#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <protocols/Wayland/RRegion.h>
#include <protocols/Wayland/RCallback.h>
#include <protocols/TearingControl/RTearingControl.h>
#include <private/LSurfacePrivate.h>
#include <LBaseSurfaceRole.h>
#include <LCompositor.h>
#include <LTime.h>
#include <LLog.h>
#include <pixman.h>

using Changes = LSurface::LSurfacePrivate::ChangesToNotify;

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

    lSurface->imp()->stateFlags.add(LSurface::LSurfacePrivate::BufferAttached);

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

    LSurface::LSurfacePrivate *imp = surface->imp();

    auto &changes = imp->changesToNotify;

    /**************************************
     *********** PENDING CHILDREN *********
     **************************************/
    imp->applyPendingChildren();

    /********************************************
     *********** NOTIFY PARENT COMMIT ***********
     ********************************************/

    for (LSurface *s : surface->children())
        if (s->role())
            s->role()->handleParentCommit();

    if (imp->stateFlags.check(LSurface::LSurfacePrivate::BufferAttached))
    {
        imp->current.buffer = imp->pending.buffer;

        if (imp->current.buffer)
            imp->stateFlags.remove(LSurface::LSurfacePrivate::BufferReleased);

        imp->stateFlags.remove(LSurface::LSurfacePrivate::BufferAttached);
    }

    // Mark the next frame as commited
    if (!imp->frameCallbacks.empty())
    {
        surface->requestedRepaint();
        for (RCallback *callback : imp->frameCallbacks)
            callback->commited = true;
    }

    /*****************************************
     *********** BUFFER TO TEXTURE ***********
     *****************************************/

    // Turn buffer into OpenGL texture and process damage
    if (imp->current.buffer)
    {
        if (!imp->stateFlags.check(LSurface::LSurfacePrivate::BufferReleased))
        {
            // Returns false on wl_client destroy
            if (!imp->bufferToTexture())
            {
                LLog::error("[RSurfacePrivate::apply_commit] Failed to convert buffer to OpenGL texture.");
                return;
            }
        }
    }

    /************************************
     *********** INPUT REGION ***********
     ************************************/
    if (surface->receiveInput())
    {
        if (imp->stateFlags.check(LSurface::LSurfacePrivate::InfiniteInput))
        {
            if (changes.check(Changes::SizeChanged))
            {
                imp->currentInputRegion.clear();
                imp->currentInputRegion.addRect(LRect(0, surface->size()));
            }
        }
        else if (changes.check(Changes::SizeChanged | Changes::InputRegionChanged))
        {
            pixman_region32_intersect_rect(&surface->imp()->currentInputRegion.m_region,
                                           &surface->imp()->pendingInputRegion.m_region,
                                           0, 0, surface->size().w(), surface->size().h());
        }
    }
    else
    {
        /******************************************
         *********** CLEAR INPUT REGION ***********
         ******************************************/
        surface->imp()->currentInputRegion.clear();
    }

    /************************************
     ********** OPAQUE REGION ***********
     ************************************/
    if (changes.check(Changes::BufferSizeChanged | Changes::SizeChanged | Changes::OpaqueRegionChanged))
    {

        if (surface->texture()->format() == DRM_FORMAT_XRGB8888)
        {
            surface->imp()->pendingOpaqueRegion.clear();
            surface->imp()->pendingOpaqueRegion.addRect(0, surface->size());
        }

        pixman_region32_intersect_rect(&surface->imp()->currentOpaqueRegion.m_region,
                                       &surface->imp()->pendingOpaqueRegion.m_region,
                                       0, 0, surface->size().w(), surface->size().h());

        /*****************************************
         ********** TRANSLUCENT REGION ***********
         *****************************************/
        pixman_box32_t box = {0, 0, surface->size().w(), surface->size().h()};
        pixman_region32_inverse(&surface->imp()->currentTranslucentRegion.m_region, &surface->imp()->currentOpaqueRegion.m_region, &box);
    }

    /*******************************************
     ***************** VSYNC *******************
     *******************************************/
    bool preferVSync = surface->surfaceResource()->imp()->rTearingControl == nullptr || surface->surfaceResource()->imp()->rTearingControl->preferVSync();

    if (imp->stateFlags.check(LSurface::LSurfacePrivate::VSync) != preferVSync)
    {
        changes.add(Changes::VSyncChanged);
        imp->stateFlags.setFlag(LSurface::LSurfacePrivate::VSync, preferVSync);
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

    if (changes.check(Changes::BufferSizeChanged))
        surface->bufferSizeChanged();

    if (changes.check(Changes::SizeChanged))
        surface->sizeChanged();

    if (changes.check(Changes::SourceRectChanged))
        surface->srcRectChanged();

    if (changes.check(Changes::BufferScaleChanged))
        surface->bufferScaleChanged();

    if (changes.check(Changes::BufferTransformChanged))
        surface->bufferTransformChanged();

    if (changes.check(Changes::DamageRegionChanged))
        surface->damageChanged();

    if (changes.check(Changes::InputRegionChanged))
        surface->inputRegionChanged();

    if (changes.check(Changes::OpaqueRegionChanged))
        surface->opaqueRegionChanged();

    if (changes.check(Changes::VSyncChanged))
        surface->preferVSyncChanged();

    changes.set(Changes::NoChanges);
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

    lSurface->imp()->pendingDamage.push_back(LRect(x, y, width, height));
    lSurface->imp()->changesToNotify.add(Changes::DamageRegionChanged);
}

void RSurface::RSurfacePrivate::set_opaque_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();

    if (region)
    {
        RRegion *rRegion = (RRegion*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingOpaqueRegion = rRegion->region();
    }
    else
        lSurface->imp()->pendingOpaqueRegion.clear();

    lSurface->imp()->changesToNotify.add(Changes::OpaqueRegionChanged);
}

void RSurface::RSurfacePrivate::set_input_region(wl_client *client, wl_resource *resource, wl_resource *region)
{
    L_UNUSED(client);

    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();

    if (region == NULL)
    {
        lSurface->imp()->pendingInputRegion.clear();
        lSurface->imp()->stateFlags.add(LSurface::LSurfacePrivate::InfiniteInput);
    }
    else
    {
        RRegion *lRRegion = (RRegion*)wl_resource_get_user_data(region);
        lSurface->imp()->pendingInputRegion = lRRegion->region();
        lSurface->imp()->stateFlags.remove(LSurface::LSurfacePrivate::InfiniteInput);
    }

    lSurface->imp()->changesToNotify.add(Changes::InputRegionChanged);
}

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
void RSurface::RSurfacePrivate::set_buffer_transform(wl_client *client, wl_resource *resource, Int32 transform)
{
    L_UNUSED(client);

    RSurface *rSurface = (RSurface*)wl_resource_get_user_data(resource);
    LSurface *lSurface = rSurface->surface();

    if (transform < 0 || transform > 7)
    {
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_TRANSFORM, "Invalid framebuffer transform %d.", transform);
        return;
    }

    lSurface->imp()->pending.transform = (LFramebuffer::Transform)transform;
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
    lSurface->imp()->pendingDamageB.push_back(LRect(x, y, width, height));
    lSurface->imp()->changesToNotify.add(Changes::DamageRegionChanged);
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
