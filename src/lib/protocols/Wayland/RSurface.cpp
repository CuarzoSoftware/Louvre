#include <cassert>
#include <protocols/SinglePixelBuffer/LSinglePixelBuffer.h>
#include <protocols/PresentationTime/RPresentationFeedback.h>
#include <protocols/TearingControl/RTearingControl.h>
#include <protocols/Wayland/GOutput.h>
#include <protocols/Wayland/RRegion.h>
#include <protocols/Wayland/RCallback.h>
#include <protocols/Wayland/GCompositor.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LSurfacePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LSeatPrivate.h>
#include <private/LFactory.h>
#include <LCursorRole.h>
#include <LDNDIconRole.h>
#include <LBackgroundBlur.h>
#include <LLog.h>

using namespace Louvre::Protocols::Wayland;

static const struct wl_surface_interface imp =
{
    .destroy                = &RSurface::destroy,
    .attach                 = &RSurface::attach,
    .damage                 = &RSurface::damage,
    .frame                  = &RSurface::frame,
    .set_opaque_region      = &RSurface::set_opaque_region,
    .set_input_region       = &RSurface::set_input_region,
    .commit                 = &RSurface::commit,

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
    .set_buffer_transform   = &RSurface::set_buffer_transform,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
    .set_buffer_scale       = &RSurface::set_buffer_scale,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 4
    .damage_buffer          = &RSurface::damage_buffer,
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    .offset                 = &RSurface::offset
#endif
};

RSurface::RSurface
(
    GCompositor *compositorRes,
    UInt32 id
)
    :LResource
    (
        compositorRes->client(),
        &wl_surface_interface,
        compositorRes->version(),
        id,
        &imp
    )
{
    // Create surface
    LSurface::Params params;
    params.surfaceResource = this;
    m_surface.reset(LFactory::createObject<LSurface>(&params));

    // Add to middle layer by default
    compositor()->imp()->layers[LLayerMiddle].emplace_back(surface());
    surface()->imp()->layerLink = std::prev(compositor()->imp()->layers[LLayerMiddle].end());

    LSurface *prev { surface()->imp()->prevSurfaceInLayers() };

    if (prev)
    {
        if (prev->nextSurface())
            surface()->imp()->compositorLink = compositor()->imp()->surfaces.insert(prev->nextSurface()->imp()->compositorLink, surface());
        else
        {
            compositor()->imp()->surfaces.emplace_back(surface());
            surface()->imp()->compositorLink = std::prev(compositor()->imp()->surfaces.end());
        }
    }
    else
    {
        compositor()->imp()->surfaces.emplace_front(surface());
        surface()->imp()->compositorLink = compositor()->imp()->surfaces.begin();
    }

    compositor()->imp()->surfacesListChanged = true;
}

RSurface::~RSurface()
{
    LSurface *lSurface { this->surface() };
    lSurface->imp()->destroyCursorOrDNDRole();

    assert(lSurface->imp()->canHostRole());

    lSurface->imp()->setKeyboardGrabToParent();

    // Notify from client
    compositor()->onAnticipatedObjectDestruction(lSurface);
    compositor()->onAnticipatedObjectDestruction(lSurface->imp()->backgroundBlur.get());

    // Unmap
    lSurface->imp()->setMapped(false);

    // Destroy pending frame callbacks
    while (!lSurface->imp()->frameCallbacks.empty())
    {
        lSurface->imp()->frameCallbacks.front()->done(LTime::ms());
        lSurface->imp()->frameCallbacks.front()->destroy();
    }

    while(!lSurface->children().empty())
        lSurface->imp()->removeChild(lSurface->imp()->children.back());

    while(!lSurface->imp()->pendingChildren.empty())
    {
        lSurface->imp()->pendingChildren.back()->imp()->pendingParent = nullptr;
        lSurface->imp()->pendingChildren.pop_back();
    }

    // Clear parent and pending parent
    lSurface->imp()->setParent(nullptr);

    // Remove the surface from the compositor list
    compositor()->imp()->surfaces.erase(lSurface->imp()->compositorLink);
    compositor()->imp()->layers[lSurface->imp()->layer].erase(lSurface->imp()->layerLink);

    compositor()->imp()->surfacesListChanged = true;
    lSurface->imp()->stateFlags.add(LSurface::LSurfacePrivate::Destroyed);
}

/******************** REQUESTS ********************/

using Changes = LSurface::LSurfacePrivate::ChangesToNotify;

void RSurface::handleOffset(LSurface *surface, Int32 x, Int32 y)
{
    if (surface->role())
        surface->role()->handleSurfaceOffset(x, y);
}

void RSurface::attach(wl_client */*client*/, wl_resource *resource, wl_resource *buffer, Int32 x, Int32 y)
{
    auto &surfaceRes { *static_cast<RSurface*>(wl_resource_get_user_data(resource)) };

    surfaceRes.surface()->imp()->stateFlags.add(LSurface::LSurfacePrivate::BufferAttached);

    if (surfaceRes.surface()->role())
        surfaceRes.surface()->role()->handleSurfaceBufferAttach(buffer, x, y);

    if (surfaceRes.surface()->imp()->pending.bufferRes)
        wl_list_remove(&surfaceRes.surface()->imp()->pending.onBufferDestroyListener.link);

    surfaceRes.surface()->imp()->pending.bufferRes = buffer;

    if (buffer)
    {
        wl_resource_add_destroy_listener(buffer, &surfaceRes.surface()->imp()->pending.onBufferDestroyListener);
        surfaceRes.surface()->imp()->pending.hasBuffer = true;
    }
    else
        surfaceRes.surface()->imp()->pending.hasBuffer = false;

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    if (surfaceRes.version() < 5)
        handleOffset(surfaceRes.surface(), x, y);
    else
        if (x != 0 || y != 0)
            surfaceRes.postError(WL_SURFACE_ERROR_INVALID_OFFSET, "Buffer offset is invalid. Check wl_surface::offset (v5).");
#else
    handleOffset(surfaceRes.surface(), x, y);
#endif
}

void RSurface::frame(wl_client *client, wl_resource *resource, UInt32 callback)
{
    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };
    new Wayland::RCallback(client, callback, &imp.frameCallbacks);
}

void RSurface::destroy(wl_client */*client*/, wl_resource *resource)
{
    auto &surfaceRes { *static_cast<RSurface*>(wl_resource_get_user_data(resource)) };

    surfaceRes.surface()->imp()->destroyCursorOrDNDRole();

    if (!surfaceRes.surface()->imp()->canHostRole())
    {
        surfaceRes.postError(WL_SURFACE_ERROR_DEFUNCT_ROLE_OBJECT, "Surface destroyed before role.");
        return;
    }

    wl_resource_destroy(resource);
}

void RSurface::commit(wl_client */*client*/, wl_resource *resource)
{
    apply_commit(static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface());
}

static bool bufferIsBeingScannedByOutputs(wl_buffer *buffer) noexcept
{
    if (!buffer)
        return false;

    for (LOutput *o : compositor()->outputs())
        if (o->imp()->scanout[0].buffer == buffer || o->imp()->scanout[1].buffer == buffer)
            return true;

    return false;
}

// The origin params indicates who requested the commit for this surface (itself or its parent surface)
void RSurface::apply_commit(LSurface *surface, LBaseSurfaceRole::CommitOrigin origin)
{
    // Check if the surface role wants to apply the commit
    if (surface->role() && !surface->role()->acceptCommitRequest(origin))
        return;

    auto &imp { *surface->imp() };

    imp.commitId++;

    for (auto *presentation : imp.presentationFeedbackResources)
        if (presentation->m_commitId == -1)
            presentation->m_commitId = imp.commitId;

    auto &changes { imp.changesToNotify };

    /**************************************
     *********** PENDING CHILDREN *********
     **************************************/
    imp.applyPendingChildren();

    /********************************************
     *********** NOTIFY PARENT COMMIT ***********
     ********************************************/

    for (LSurface *s : surface->children())
        s->imp()->stateFlags.remove(LSurface::LSurfacePrivate::ParentCommitNotified);

    retryParentCommitNotif:
    imp.stateFlags.remove(LSurface::LSurfacePrivate::ChildrenListChanged);

    for (LSurface *s : surface->children())
    {
        if (!s->imp()->stateFlags.check(LSurface::LSurfacePrivate::ParentCommitNotified))
        {
            s->imp()->stateFlags.add(LSurface::LSurfacePrivate::ParentCommitNotified);

            if (s->role())
                s->role()->handleParentCommit();

            if (imp.stateFlags.check(LSurface::LSurfacePrivate::ChildrenListChanged))
                goto retryParentCommitNotif;
        }
    }

    if (imp.stateFlags.check(LSurface::LSurfacePrivate::BufferAttached))
    {
        if (imp.current.bufferRes)
        {
            wl_list_remove(&imp.current.onBufferDestroyListener.link);

            /* Release WL_DRM and DMA buffers only if a seccond buffer has been attached.
             * Also, if being scanned out, let outputs take care of releasing them.
             * SHM and Single Pixel buffers are released in LSurface::LSurfacePrivate::bufferToTexture() */
            if (!bufferIsBeingScannedByOutputs((wl_buffer*)imp.current.bufferRes)
                && !wl_shm_buffer_get(imp.current.bufferRes)
                && !LSinglePixelBuffer::isSinglePixelBuffer(imp.current.bufferRes)
                && imp.current.bufferRes != imp.pending.bufferRes)
            {
                wl_buffer_send_release(imp.current.bufferRes);
                wl_client_flush(wl_resource_get_client(imp.current.bufferRes));
            }
        }

        imp.current.hasBuffer = imp.pending.hasBuffer;
        imp.current.bufferRes = imp.pending.bufferRes;

        if (imp.current.bufferRes)
        {
            wl_resource_add_destroy_listener(imp.current.bufferRes, &imp.current.onBufferDestroyListener);
            imp.stateFlags.remove(LSurface::LSurfacePrivate::BufferReleased);
        }

        imp.stateFlags.remove(LSurface::LSurfacePrivate::BufferAttached);
    }

    // Mark the next frame as commited
    if (!imp.frameCallbacks.empty())
    {
        for (RCallback *callback : imp.frameCallbacks)
            callback->m_commited = true;

        surface->requestedRepaint();
    }

    /*****************************************
     *********** BUFFER TO TEXTURE ***********
     *****************************************/

    // Turn buffer into OpenGL texture and process damage
    if (imp.current.hasBuffer)
    {
        // Returns false on wl_client destroy
        if (!imp.bufferToTexture())
        {
            LLog::error("[RSurface::apply_commit] Failed to convert buffer to OpenGL texture.");
            return;
        }
    }

    /************************************
     *********** INPUT REGION ***********
     ************************************/
    if (surface->receiveInput())
    {
        if (imp.stateFlags.check(LSurface::LSurfacePrivate::InfiniteInput))
        {
            if (changes.check(Changes::SizeChanged | Changes::InputRegionChanged))
            {
                imp.currentInputRegion.clear();
                imp.currentInputRegion.addRect(0, 0, surface->size());
                changes.add(Changes::InputRegionChanged);
            }
        }
        else if (changes.check(Changes::SizeChanged | Changes::InputRegionChanged))
        {
            pixman_region32_intersect_rect(&imp.currentInputRegion.m_region,
                                           &imp.pendingInputRegion.m_region,
                                           0, 0, surface->size().w(), surface->size().h());
            changes.add(Changes::InputRegionChanged);
        }
    }
    else
    {
        /******************************************
         *********** CLEAR INPUT REGION ***********
         ******************************************/
        imp.currentInputRegion.clear();
        imp.pendingPointerConstraintRegion.reset();
        imp.pointerConstraintRegion.clear();
    }

    /******************************************
     *********** POINTER CONSTRAINT ***********
     ******************************************/

    if (surface->pointerConstraintMode() != LSurface::Free)
    {
        if (changes.check(Changes::PointerConstraintRegionChanged | Changes::InputRegionChanged))
        {
            if (imp.pendingPointerConstraintRegion)
            {
                imp.pointerConstraintRegion = *imp.pendingPointerConstraintRegion;
                imp.pointerConstraintRegion.intersectRegion(imp.currentInputRegion);
            }
            else
                imp.pointerConstraintRegion = imp.currentInputRegion;

            changes.add(Changes::PointerConstraintRegionChanged);
        }

        if (changes.check(Changes::LockedPointerPosHintChanged))
            imp.current.lockedPointerPosHint = imp.pending.lockedPointerPosHint;
    }

    /****************************************
     *********** INVISIBLE REGION ***********
     ****************************************/
    if (imp.stateFlags.check(LSurface::LSurfacePrivate::InfiniteInvisible))
    {
        if (changes.check(Changes::SizeChanged | Changes::InvisibleRegionChanged))
        {
            imp.currentInvisibleRegion.clear();
            imp.currentInvisibleRegion.addRect(0, 0, surface->size());
            changes.add(Changes::InvisibleRegionChanged);
        }
    }
    else if (changes.check(Changes::SizeChanged | Changes::InvisibleRegionChanged))
    {
        pixman_region32_intersect_rect(&imp.currentInvisibleRegion.m_region,
                                       &imp.pendingInvisibleRegion.m_region,
                                       0, 0, surface->size().w(), surface->size().h());
        changes.add(Changes::InvisibleRegionChanged);
    }

    /************************************
     ********** OPAQUE REGION ***********
     ************************************/
    if (changes.check(Changes::BufferSizeChanged | Changes::SizeChanged | Changes::OpaqueRegionChanged))
    {
        /*
        if (surface->texture()->format() == DRM_FORMAT_XRGB8888)
        {
            imp.pendingOpaqueRegion.clear();
            imp.pendingOpaqueRegion.addRect(0, 0, surface->size());
        }*/

        pixman_region32_intersect_rect(&imp.currentOpaqueRegion.m_region,
                                       &imp.pendingOpaqueRegion.m_region,
                                       0, 0, surface->size().w(), surface->size().h());

        /*****************************************
         ********** TRANSLUCENT REGION ***********
         *****************************************/
        pixman_box32_t box {0, 0, surface->size().w(), surface->size().h()};
        pixman_region32_inverse(&imp.currentTranslucentRegion.m_region, &imp.currentOpaqueRegion.m_region, &box);
    }

    /*******************************************
     ***************** VSYNC *******************
     *******************************************/
    const bool preferVSync { surface->surfaceResource()->tearingControlRes() == nullptr || surface->surfaceResource()->tearingControlRes()->preferVSync() };

    if (imp.stateFlags.check(LSurface::LSurfacePrivate::VSync) != preferVSync)
    {
        changes.add(Changes::VSyncChanged);
        imp.stateFlags.setFlag(LSurface::LSurfacePrivate::VSync, preferVSync);
    }

    /**************************************************
     ***************** CONTENT TYPE *******************
     **************************************************/
    if (imp.pending.contentType != imp.current.contentType)
    {
        changes.add(Changes::ContentTypeChanged);
        imp.current.contentType = imp.pending.contentType;
    }

    LWeak<LSurface> ref { surface };

    /*******************************************
     *********** NOTIFY COMMIT TO ROLE *********
     *******************************************/
    if (surface->role())
        surface->role()->handleSurfaceCommit(origin);

    if (!ref)
        return;

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

    if (changes.check(Changes::PointerConstraintRegionChanged))
        surface->pointerConstraintRegionChanged();

    if (changes.check(Changes::LockedPointerPosHintChanged))
        surface->lockedPointerPosHintChanged();

    if (changes.check(Changes::OpaqueRegionChanged))
        surface->opaqueRegionChanged();

    if (changes.check(Changes::InvisibleRegionChanged))
        surface->invisibleRegionChanged();

    if (changes.check(Changes::VSyncChanged))
        surface->preferVSyncChanged();

    if (changes.check(Changes::ContentTypeChanged))
        surface->contentTypeChanged();

    surface->backgroundBlur()->handleCommit(changes.check(Changes::SizeChanged));

    changes.set(Changes::NoChanges);
}

void RSurface::damage(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };

    // Ignore rects with invalid or crazy sizes
    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    if (height <= 0)
        return;

    imp.pendingDamage.emplace_back(x, y, width, height);
    imp.changesToNotify.add(Changes::DamageRegionChanged);
}

void RSurface::set_opaque_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };

    if (region)
    {
        imp.pendingOpaqueRegion = static_cast<const RRegion*>(wl_resource_get_user_data(region))->region();
        imp.changesToNotify.add(Changes::OpaqueRegionChanged);
    }
    else if (!imp.pendingOpaqueRegion.empty())
    {
        imp.pendingOpaqueRegion.clear();
        imp.changesToNotify.add(Changes::OpaqueRegionChanged);
    }
}

void RSurface::set_input_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };

    if (region)
    {
        imp.pendingInputRegion = static_cast<const RRegion*>(wl_resource_get_user_data(region))->region();
        imp.stateFlags.remove(LSurface::LSurfacePrivate::InfiniteInput);
        imp.changesToNotify.add(Changes::InputRegionChanged);
    }
    else if (!imp.stateFlags.check(LSurface::LSurfacePrivate::InfiniteInput))
    {
        imp.pendingInputRegion.clear();
        imp.stateFlags.add(LSurface::LSurfacePrivate::InfiniteInput);
        imp.changesToNotify.add(Changes::InputRegionChanged);
    }
}

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
void RSurface::set_buffer_transform(wl_client */*client*/, wl_resource *resource, Int32 transform)
{
    auto &res { *static_cast<RSurface*>(wl_resource_get_user_data(resource)) };
    if (transform < 0 || transform > 7)
    {
        res.postError(WL_SURFACE_ERROR_INVALID_TRANSFORM, "Invalid framebuffer transform %d.", transform);
        return;
    }

    auto &imp { *res.surface()->imp() };
    imp.pending.transform = static_cast<LTransform>(transform);
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
void RSurface::set_buffer_scale(wl_client */*client*/, wl_resource *resource, Int32 scale)
{
    auto &res { *static_cast<RSurface*>(wl_resource_get_user_data(resource)) };

    if (scale <= 0)
    {
        res.postError(WL_SURFACE_ERROR_INVALID_SCALE, "Buffer scale must be >= 1.");
        return;
    }

    auto &imp { *res.surface()->imp() };
    imp.pending.bufferScale = scale;
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 4
void RSurface::damage_buffer(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y, Int32 width, Int32 height)
{
    // Ignore rects with invalid or crazy sizes

    if (width > LOUVRE_MAX_SURFACE_SIZE)
        width = LOUVRE_MAX_SURFACE_SIZE;
    if (width <= 0)
        return;

    if (height > LOUVRE_MAX_SURFACE_SIZE)
        height = LOUVRE_MAX_SURFACE_SIZE;
    if (height <= 0)
        return;

    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };
    imp.pendingDamageB.emplace_back(x, y, width, height);
    imp.changesToNotify.add(Changes::DamageRegionChanged);
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
void RSurface::offset(wl_client */*client*/, wl_resource *resource, Int32 x, Int32 y)
{
    handleOffset(static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface(),
                 x,
                 y);
}
#endif

/******************** EVENTS ********************/

void RSurface::enter(GOutput *outputRes) noexcept
{
    wl_surface_send_enter(resource(), outputRes->resource());
}

void RSurface::leave(GOutput *outputRes) noexcept
{
    wl_surface_send_leave(resource(), outputRes->resource());
}

bool RSurface::preferredBufferScale(Int32 scale) noexcept
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

bool RSurface::preferredBufferTransform(LTransform transform) noexcept
{
#if LOUVRE_WL_COMPOSITOR_VERSION >= 6
    if (version() >= 6)
    {
        wl_surface_send_preferred_buffer_transform(resource(), static_cast<UInt32>(transform));
        return true;
    }
#endif
    L_UNUSED(transform);
    return false;
}
