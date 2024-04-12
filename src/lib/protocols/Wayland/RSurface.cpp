#include <protocols/PresentationTime/RPresentationFeedback.h>
#include <protocols/TearingControl/RTearingControl.h>
#include <protocols/Wayland/GOutput.h>
#include <protocols/Wayland/RRegion.h>
#include <protocols/Wayland/RCallback.h>
#include <protocols/Wayland/GCompositor.h>
#include <protocols/Wayland/RSurface.h>
#include <private/LSurfacePrivate.h>
#include <private/LSeatPrivate.h>
#include <LCursorRole.h>
#include <LDNDIconRole.h>
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
    m_surface.reset(compositor()->createSurfaceRequest(&params));

    // Append surface
    compositor()->imp()->surfaces.emplace_back(surface());
    surface()->imp()->compositorLink = std::prev(compositor()->imp()->surfaces.end());
    compositor()->imp()->surfacesListChanged = true;
}

RSurface::~RSurface()
{
    LSurface *lSurface { this->surface() };

    lSurface->imp()->setKeyboardGrabToParent();

    // Notify from client
    compositor()->destroySurfaceRequest(lSurface);

    // Unmap
    lSurface->imp()->setMapped(false);

    // Destroy pending frame callbacks
    while (!lSurface->imp()->frameCallbacks.empty())
        lSurface->imp()->frameCallbacks.front()->destroy();

    // Clear active toplevel focus
    if (seat()->imp()->activeToplevel == lSurface->toplevel())
        seat()->imp()->activeToplevel = nullptr;

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
    const auto &surfaceRes { *static_cast<const RSurface*>(wl_resource_get_user_data(resource)) };

    surfaceRes.surface()->imp()->stateFlags.add(LSurface::LSurfacePrivate::BufferAttached);

    if (surfaceRes.surface()->role())
        surfaceRes.surface()->role()->handleSurfaceBufferAttach(buffer, x, y);

    if (surfaceRes.surface()->imp()->pending.buffer)
        wl_list_remove(&surfaceRes.surface()->imp()->pending.onBufferDestroyListener.link);

    surfaceRes.surface()->imp()->pending.buffer = buffer;

    if (buffer)
        wl_resource_add_destroy_listener(buffer, &surfaceRes.surface()->imp()->pending.onBufferDestroyListener);

#if LOUVRE_WL_COMPOSITOR_VERSION >= 5
    if (surfaceRes.version() < 5)
        handleOffset(surfaceRes.surface(), x, y);
    else
        if (x != 0 || y != 0)
            wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_OFFSET, "Buffer offset is invalid. Check wl_surface::offset (v5).");
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
    wl_resource_destroy(resource);
}

void RSurface::commit(wl_client */*client*/, wl_resource *resource)
{
    apply_commit(static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface());
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
        if (s->role())
            s->role()->handleParentCommit();

    if (imp.stateFlags.check(LSurface::LSurfacePrivate::BufferAttached))
    {
        if (imp.current.buffer)
            wl_list_remove(&imp.current.onBufferDestroyListener.link);

        imp.current.buffer = imp.pending.buffer;

        if (imp.current.buffer)
        {
            wl_resource_add_destroy_listener(imp.current.buffer, &imp.current.onBufferDestroyListener);
            imp.stateFlags.remove(LSurface::LSurfacePrivate::BufferReleased);
        }

        imp.stateFlags.remove(LSurface::LSurfacePrivate::BufferAttached);
    }

    // Mark the next frame as commited
    if (!imp.frameCallbacks.empty())
    {
        surface->requestedRepaint();
        for (RCallback *callback : imp.frameCallbacks)
            callback->m_commited = true;
    }

    /*****************************************
     *********** BUFFER TO TEXTURE ***********
     *****************************************/

    // Turn buffer into OpenGL texture and process damage
    if (imp.current.buffer)
    {
        if (!imp.stateFlags.check(LSurface::LSurfacePrivate::BufferReleased))
        {
            // Returns false on wl_client destroy
            if (!imp.bufferToTexture())
            {
                LLog::error("[RSurface::apply_commit] Failed to convert buffer to OpenGL texture.");
                return;
            }
        }
    }

    /************************************
     *********** INPUT REGION ***********
     ************************************/
    if (surface->receiveInput())
    {
        if (imp.stateFlags.check(LSurface::LSurfacePrivate::InfiniteInput))
        {
            if (changes.check(Changes::SizeChanged))
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

    /************************************
     ********** OPAQUE REGION ***********
     ************************************/
    if (changes.check(Changes::BufferSizeChanged | Changes::SizeChanged | Changes::OpaqueRegionChanged))
    {

        if (surface->texture()->format() == DRM_FORMAT_XRGB8888)
        {
            imp.pendingOpaqueRegion.clear();
            imp.pendingOpaqueRegion.addRect(0, 0, surface->size());
        }

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

    LWeak<LSurface> ref { surface };

    /*******************************************
     *********** NOTIFY COMMIT TO ROLE *********
     *******************************************/
    if (surface->role())
        surface->role()->handleSurfaceCommit(origin);
    else if (imp.pending.role)
        imp.pending.role->handleSurfaceCommit(origin);

    if (changes.check(Changes::BufferSizeChanged))
    {
        surface->bufferSizeChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::SizeChanged))
    {
        surface->sizeChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::SourceRectChanged))
    {
        surface->srcRectChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::BufferScaleChanged))
    {
        surface->bufferScaleChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::BufferTransformChanged))
    {
        surface->bufferTransformChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::DamageRegionChanged))
    {
        surface->damageChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::InputRegionChanged))
    {
        surface->inputRegionChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::PointerConstraintRegionChanged))
    {
        surface->pointerConstraintRegionChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::LockedPointerPosHintChanged))
    {
        surface->lockedPointerPosHintChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::OpaqueRegionChanged))
    {
        surface->opaqueRegionChanged();

        if (!ref)
            return;
    }

    if (changes.check(Changes::VSyncChanged))
    {
        surface->preferVSyncChanged();

        if (!ref)
            return;
    }

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
        imp.pendingOpaqueRegion = static_cast<const RRegion*>(wl_resource_get_user_data(region))->region();
    else
        imp.pendingOpaqueRegion.clear();

    imp.changesToNotify.add(Changes::OpaqueRegionChanged);
}

void RSurface::set_input_region(wl_client */*client*/, wl_resource *resource, wl_resource *region)
{
    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };

    if (region)
    {
        imp.pendingInputRegion = static_cast<const RRegion*>(wl_resource_get_user_data(region))->region();
        imp.stateFlags.remove(LSurface::LSurfacePrivate::InfiniteInput);
    }
    else
    {
        imp.pendingInputRegion.clear();
        imp.stateFlags.add(LSurface::LSurfacePrivate::InfiniteInput);
    }

    imp.changesToNotify.add(Changes::InputRegionChanged);
}

#if LOUVRE_WL_COMPOSITOR_VERSION >= 2
void RSurface::set_buffer_transform(wl_client */*client*/, wl_resource *resource, Int32 transform)
{
    if (transform < 0 || transform > 7)
    {
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_TRANSFORM, "Invalid framebuffer transform %d.", transform);
        return;
    }

    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };
    imp.pending.transform = static_cast<LFramebuffer::Transform>(transform);
}
#endif

#if LOUVRE_WL_COMPOSITOR_VERSION >= 3
void RSurface::set_buffer_scale(wl_client */*client*/, wl_resource *resource, Int32 scale)
{
    if (scale <= 0)
    {
        wl_resource_post_error(resource, WL_SURFACE_ERROR_INVALID_SCALE, "Buffer scale must be >= 1.");
        return;
    }

    auto &imp { *static_cast<const RSurface*>(wl_resource_get_user_data(resource))->surface()->imp() };
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

bool RSurface::preferredBufferTransform(UInt32 transform) noexcept
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
