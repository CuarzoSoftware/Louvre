#include <CZ/Louvre/Protocols/PresentationTime/presentation-time.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/LSinglePixelBuffer.h>
#include <CZ/Louvre/Protocols/FractionalScale/RFractionalScale.h>
#include <CZ/Louvre/Protocols/TearingControl/RTearingControl.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/linux-drm-syncobj-v1.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/RDRMSyncObjSurface.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/Protocols/Wayland/RWlSurface.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LKeyboardPrivate.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/Roles/LPopupRole.h>
#include <CZ/Louvre/Roles/LLayerRole.h>
#include <CZ/Louvre/Roles/LSurfaceLock.h>
#include <CZ/Louvre/Roles/LBackgroundBlur.h>
#include <CZ/Louvre/Seat/LOutputMode.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LLog.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Ream/WL/RWLFormat.h>
#include <CZ/Ream/DRM/RDRMTimeline.h>
#include <CZ/Ream/RSync.h>

#include <CZ/Core/Utils/CZRegionUtils.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Core/CZCore.h>

#include <cassert>

using Changes = LSurfaceCommitEvent::Changes;

void LSurface::LSurfacePrivate::setMapped(bool state, bool notifyLater) noexcept
{
    if (stateFlags.has(Destroyed))
        return;

    LSurface *surface { surfaceResource->surface() };

    if (stateFlags.has(Mapped) != state)
    {
        stateFlags.setFlag(Mapped, state);

        if (notifyLater)
            current.changesToNotify.add(Changes::MappingChanged);
        else
            surface->mappingChanged();

        for (auto *sub : current.subsurfacesBelow)
            sub->handleParentMappingChange();

        for (auto *sub : current.subsurfacesAbove)
            sub->handleParentMappingChange();
    }
}

void LSurface::LSurfacePrivate::setRole(LBaseSurfaceRole *role, bool notify) noexcept
{
    if (this->role == role)
        return;

    this->role = role;

    if (notify)
        surfaceResource->surface()->roleChanged();
}

bool LSurface::LSurfacePrivate::bufferToImage(Uncommitted &pending) noexcept
{
    auto ream { RCore::Get() };

    // Size of the current buffer without transform
    Int32 widthB, heightB;

    /***************************************
     *********** BUFFER TRANSFOM ***********
     ***************************************/

    if (current.transform != pending.transform)
    {
        current.transform = pending.transform;
        current.changesToNotify.add(Changes::BufferTransformChanged);
    }

    /***********************************
     *********** BUFFER SCALE ***********
     ***********************************/

    if (current.scale != pending.scale)
    {
        current.scale = pending.scale;
        current.changesToNotify.add(Changes::BufferScaleChanged);
    }

    if (current.buffer.buffer.res())
    {
        // SHM
        if (wl_shm_buffer_get(current.buffer.buffer.res()))
        {
            wl_shm_buffer *shm_buffer { wl_shm_buffer_get(current.buffer.buffer.res()) };
            wl_shm_buffer_begin_access(shm_buffer);
            auto *pixels { (UInt8*)wl_shm_buffer_get_data(shm_buffer) };
            const auto format { RWLFormat::ToDRM((wl_shm_format)wl_shm_buffer_get_format(shm_buffer)) };
            const auto stride { wl_shm_buffer_get_stride(shm_buffer) };
            widthB = wl_shm_buffer_get_width(shm_buffer);
            heightB = wl_shm_buffer_get_height(shm_buffer);

            if (!updateDimensions(widthB, heightB))
                return false;

            if (!current.image || !stateFlags.has(CurrentImageIsSHM) || current.changesToNotify.has(Changes::SizeChanged | Changes::SrcRectChanged | Changes::BufferSizeChanged | Changes::BufferTransformChanged | Changes::BufferScaleChanged))
            {
                current.bufferDamage.setRect(SkIRect::MakeSize(sizeB));
                current.damage.setRect(SkIRect::MakeSize(size));

                auto fmt { ream->mainDevice()->textureFormats().formats().find(format) };

                if (fmt == ream->mainDevice()->textureFormats().formats().end())
                {
                    current.image.reset();
                    current.buffer.weakImage.reset();
                }
                else
                {
                    RPixelBufferInfo info {};
                    info.format = format;
                    info.pixels = pixels;
                    info.alphaType = kUnknown_SkAlphaType;
                    info.stride = stride;
                    info.size.set(widthB, heightB);

                    RImageConstraints cons {};
                    cons.allocator = ream->mainDevice();
                    cons.caps[cons.allocator] = RImageCap_Src;
                    cons.writeFormats.emplace(format);

                    current.buffer.weakImage = current.image = RImage::MakeFromPixels(info, *fmt, &cons);
                    stateFlags.add(CurrentImageIsSHM);
                }
            }
            else if (!pending.bufferDamage.empty() || !pending.damage.empty())
            {
                SkRegion onlyPending;

                if (stateFlags.has(ViewportIsScaled | ViewportIsCropped))
                {
                    Float32 xInvScale = (Float32(current.scale) * srcRect.width())/Float32(size.width());
                    Float32 yInvScale = (Float32(current.scale) * srcRect.height())/Float32(size.height());

                    Int32 xOffset = roundf(srcRect.x() * Float32(current.scale)) - 2;
                    Int32 yOffset = roundf(srcRect.y() * Float32(current.scale)) - 2;

                    while (!pending.damage.empty())
                    {
                        SkIRect &r = pending.damage.back();
                        onlyPending.op(
                            SkIRect::MakeXYWH(
                                (r.x() * xInvScale + xOffset),
                                (r.y() * yInvScale + yOffset),
                                (r.width() * xInvScale + 4 ),
                                (r.height() * yInvScale + 4 )),
                            SkRegion::Op::kUnion_Op);
                        pending.damage.pop_back();
                    }

                    while (!pending.bufferDamage.empty())
                    {
                        onlyPending.op(pending.bufferDamage.back().makeOutset(1, 1), SkRegion::Op::kUnion_Op);
                        pending.bufferDamage.pop_back();
                    }

                    CZRegionUtils::ApplyTransform(onlyPending, sizeB, current.transform);

                    if (!onlyPending.isEmpty())
                    {
                        RPixelBufferRegion info {};
                        info.pixels = pixels;
                        info.region = onlyPending;
                        info.stride = stride;
                        info.format = format;
                        current.image->writePixels(info);
                    }

                    CZRegionUtils::ApplyTransform(onlyPending, sizeB, CZ::RequiredTransform(current.transform, CZTransform::Normal));
                    current.bufferDamage.op(onlyPending, SkRegion::Op::kUnion_Op);
                    current.damage = current.bufferDamage;
                    current.damage.translate(-xOffset - 2, -yOffset - 2);
                    CZRegionUtils::Scale(current.damage, 1.f/xInvScale, 1.f/yInvScale);
                }
                else
                {
                    while (!pending.damage.empty())
                    {
                        SkIRect &r = pending.damage.back();
                        onlyPending.op(
                            SkIRect::MakeXYWH(
                                (r.x() - 2)*current.scale,
                                (r.y() - 2)*current.scale,
                                (r.width() + 4 )*current.scale,
                                (r.height() + 4 )*current.scale),
                            SkRegion::Op::kUnion_Op);
                        pending.damage.pop_back();
                    }

                    while (!pending.bufferDamage.empty())
                    {
                        onlyPending.op(pending.bufferDamage.back().makeOutset(2, 2), SkRegion::Op::kUnion_Op);
                        pending.bufferDamage.pop_back();
                    }

                    onlyPending.op(SkIRect::MakeSize(sizeB), SkRegion::kIntersect_Op);
                    current.bufferDamage.op(onlyPending, SkRegion::kUnion_Op);
                    CZRegionUtils::ApplyTransform(onlyPending, sizeB, current.transform);

                    if (!onlyPending.isEmpty())
                    {

                        RPixelBufferRegion info {};
                        info.pixels = pixels;
                        info.region = onlyPending;
                        info.stride = stride;
                        info.format = format;
                        current.image->writePixels(info);
                    }

                    CZRegionUtils::Scale(current.bufferDamage, current.damage, 1.f/Float32(current.scale));
                }
            }
            else
            {
                wl_shm_buffer_end_access(shm_buffer);
                current.buffer.release();
                wl_client_flush(wl_resource_get_client(current.buffer.buffer.res()));
                return true;
            }

            wl_shm_buffer_end_access(shm_buffer);
            current.buffer.release();
            wl_client_flush(wl_resource_get_client(current.buffer.buffer.res()));
        }

        // DMA-Buf
        else if (LDMABuffer::isDMABuffer(current.buffer.buffer.res()))
        {
            /* Unlike SHM buffers, the current buffer is released after
             * a different buffer is commited */

            LDMABuffer *dmaBuffer = (LDMABuffer*)wl_resource_get_user_data(current.buffer.buffer.res());
            current.buffer.weakImage = current.image = dmaBuffer->image();
            stateFlags.remove(CurrentImageIsSHM);

            if (!current.image)
            {
                surfaceResource->postError(0, "Unknown buffer type.");
                return false;
            }

            if (current.buffer.acquireTimeline)
            {
                current.image->setWriteSync(
                    RSync::FromExternal(current.buffer.acquireTimeline->exportSyncFile(current.buffer.acquirePoint).release()));
            }

            widthB = current.image->size().width();
            heightB = current.image->size().height();

            if (!updateDimensions(widthB, heightB))
                return false;

            updateDamage(pending);
        }
        // Single pixel buffer
        else if (LSinglePixelBuffer::isSinglePixelBuffer(current.buffer.buffer.res()))
        {
            LSinglePixelBuffer &singlePixelBuffer { *static_cast<LSinglePixelBuffer*>(wl_resource_get_user_data(current.buffer.buffer.res())) };
            current.buffer.weakImage = current.image = singlePixelBuffer.image;
            stateFlags.remove(CurrentImageIsSHM);

            if (!current.image)
            {
                surfaceResource->postError(0, "Unknown buffer type.");
                return false;
            }

            widthB = heightB = 1;

            if (!updateDimensions(widthB, heightB))
                return false;

            updateDamage(pending);
            current.buffer.release();
        }
        else
        {
            LLog(CZError, CZLN, "Unknown buffer type. Killing client");
            surfaceResource->postError(0, "Unknown buffer type.");
            return false;
        }
    }
    else
    {
        if (!current.image)
            return false;

        widthB = current.image->size().width();
        heightB = current.image->size().height();

        if (!updateDimensions(widthB, heightB))
            return false;

        updateDamage(pending);
    }

    //texture->m_surface.reset(surfaceResource->surface());
    pending.bufferDamage.clear();
    pending.damage.clear();
    damageId++;
    stateFlags.add(Damaged);
    return true;
}

void LSurface::LSurfacePrivate::sendPreferredScale() noexcept
{
    if (outputs.empty())
        return;

    Int32 wlScale { 0 };
    Float32 wlFracScale { 0.f };
    CZTransform transform { CZTransform::Normal };

    for (LOutput *o : outputs)
    {
        if (o->imp()->fractionalScale > wlFracScale)
        {
            wlScale = o->imp()->scale;
            wlFracScale = o->imp()->fractionalScale;
            transform = o->transform();
        }
    }

    if (lastSentPreferredBufferScale != wlScale)
    {
        lastSentPreferredBufferScale = wlScale;
        surfaceResource->preferredBufferScale(lastSentPreferredBufferScale);
    }

    if (lastSentPreferredTransform != transform)
    {
        lastSentPreferredTransform = transform;
        surfaceResource->preferredBufferTransform(lastSentPreferredTransform);
    }

    if (surfaceResource->fractionalScaleRes())
        surfaceResource->fractionalScaleRes()->preferredScale(wlFracScale);
}

bool LSurface::LSurfacePrivate::hasBufferOrPendingBuffer() noexcept
{
    return current.buffer.buffer.res() || pending.buffer.buffer.res();
}

void LSurface::LSurfacePrivate::setKeyboardGrabToParent()
{
    /* TODO:
    if (seat()->keyboard()->grab() == surfaceResource->surface())
    {
        if (surfaceResource->surface()->parent())
        {
            if (surfaceResource->surface()->parent()->popup())
                seat()->keyboard()->setGrab(surfaceResource->surface()->parent());
            else
            {
                seat()->keyboard()->imp()->grab.reset();
                seat()->keyboard()->imp()->focus.reset();
                seat()->keyboard()->setFocus(surfaceResource->surface()->parent());
            }
        }
        else
            seat()->keyboard()->setGrab(nullptr);
    }
    */
}

void LSurface::LSurfacePrivate::updateDamage(Uncommitted &pending) noexcept
{
    if (!current.image || current.changesToNotify.has(Changes::SizeChanged | Changes::SrcRectChanged | Changes::BufferSizeChanged | Changes::BufferTransformChanged | Changes::BufferScaleChanged))
    {
        current.damage.setRect(SkIRect::MakeSize(size));
    }
    else if (!pending.bufferDamage.empty() || !pending.damage.empty())
    {
        if (stateFlags.has(ViewportIsScaled | ViewportIsCropped))
        {
            Float32 xInvScale = (Float32(current.scale) * srcRect.width())/Float32(size.width());
            Float32 yInvScale = (Float32(current.scale) * srcRect.height())/Float32(size.height());

            Int32 xOffset = roundf(srcRect.x() * Float32(current.scale)) - 2;
            Int32 yOffset = roundf(srcRect.y() * Float32(current.scale)) - 2;

            while (!pending.damage.empty())
            {
                SkIRect &r = pending.damage.back();
                current.bufferDamage.op(
                    SkIRect::MakeXYWH(
                        (r.x() * xInvScale + xOffset),
                        (r.y() * yInvScale + yOffset),
                        (r.width() * xInvScale + 4 ),
                        (r.height() * yInvScale + 4 )),
                    SkRegion::Op::kUnion_Op);
                pending.damage.pop_back();
            }

            while (!pending.bufferDamage.empty())
            {
                SkIRect &r = pending.bufferDamage.back();
                current.bufferDamage.op(r.makeOutset(1, 1), SkRegion::Op::kUnion_Op);
                pending.bufferDamage.pop_back();
            }

            current.bufferDamage.op(SkIRect::MakeSize(sizeB), SkRegion::Op::kIntersect_Op);
            current.damage = current.bufferDamage;
            current.damage.translate(-xOffset - 2, -yOffset - 2);
            CZRegionUtils::Scale(current.damage, 1.f/xInvScale, 1.f/yInvScale);
        }
        else
        {
            while (!pending.damage.empty())
            {
                SkIRect &r = pending.damage.back();
                current.bufferDamage.op(
                    SkIRect::MakeXYWH(
                        (r.x() - 1 )*current.scale,
                        (r.y() - 1 )*current.scale,
                        (r.width() + 2 )*current.scale,
                        (r.height() + 2 )*current.scale),
                    SkRegion::Op::kUnion_Op);
                pending.damage.pop_back();
            }

            while (!pending.bufferDamage.empty())
            {
                SkIRect &r = pending.bufferDamage.back();
                current.bufferDamage.op(r.makeOutset(1, 1), SkRegion::Op::kUnion_Op);
                pending.bufferDamage.pop_back();
            }

            current.bufferDamage.op(SkIRect::MakeSize(sizeB), SkRegion::Op::kIntersect_Op);
            CZRegionUtils::Scale(current.bufferDamage, current.damage, 1.f/Float32(current.scale));
        }
    }
}

bool LSurface::LSurfacePrivate::updateDimensions(Int32 widthB, Int32 heightB) noexcept
{
    const SkISize prevSizeB { sizeB };
    const SkISize prevSize { size };
    const SkRect prevSrcRect { srcRect };

    if (CZ::Is90Transform(current.transform))
    {
        sizeB.fWidth = heightB;
        sizeB.fHeight = widthB;
    }
    else
    {
        sizeB.fWidth = widthB;
        sizeB.fHeight = heightB;
    }

    if (prevSizeB != sizeB)
        current.changesToNotify.add(Changes::BufferSizeChanged);

    if (surfaceResource->viewportRes())
    {
        bool usingViewportSrc { false };

        // Using the viewport source rect
        if (surfaceResource->viewportRes()->srcRect().x() != -1.f ||
            surfaceResource->viewportRes()->srcRect().y() != -1.f ||
            surfaceResource->viewportRes()->srcRect().width() != -1.f ||
            surfaceResource->viewportRes()->srcRect().height() != -1.f)
        {
            usingViewportSrc = true;

            srcRect = surfaceResource->viewportRes()->srcRect();

            if (srcRect.x() < 0.f || srcRect.y() < 0.f || srcRect.width() <= 0.f || srcRect.height() <= 0.f)
            {
                surfaceResource->viewportRes()->postError(
                   WP_VIEWPORT_ERROR_BAD_VALUE,
                   "Invalid source rect ({}, {}, {}, {}).",
                   srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
                return false;
            }

            if (roundf((srcRect.x() + srcRect.width()) * Float32(current.scale)) > sizeB.width() || roundf((srcRect.y() + srcRect.height()) * Float32(current.scale)) > sizeB.height())
            {
                surfaceResource->viewportRes()->postError(
                    WP_VIEWPORT_ERROR_OUT_OF_BUFFER,
                    "Source rectangle extends outside of the content area rect.");
                return false;
            }

            stateFlags.add(ViewportIsCropped);
        }

        // Using the entire buffer as source
        else
        {
            srcRect.setWH(
                Float32(sizeB.width()) / Float32(current.scale),
                Float32(sizeB.height()) / Float32(current.scale));
            stateFlags.remove(ViewportIsCropped);
        }

        // Using the viewport destination size
        if (surfaceResource->viewportRes()->dstSize().width() != -1 || surfaceResource->viewportRes()->dstSize().height() != -1)
        {
            size = surfaceResource->viewportRes()->dstSize();

            if (size.width() <= 0 || size.height() <= 0)
            {
                surfaceResource->viewportRes()->postError(
                    WP_VIEWPORT_ERROR_BAD_VALUE,
                    "Invalid destination size ({}, {}).",
                    size.width(), size.height());
                return false;
            }

            stateFlags.add(ViewportIsScaled);
        }

        // Using the viewport source rect size or normal surface size
        else
        {
            if (usingViewportSrc)
            {
                if (fmod(srcRect.width(), 1.f) != 0.f || fmod(srcRect.height(), 1.f) != 0.f)
                {
                    surfaceResource->viewportRes()->postError(
                        WP_VIEWPORT_ERROR_BAD_SIZE,
                        "Destination size is not integer");
                    return false;
                }

                size.fWidth = srcRect.width();
                size.fHeight = srcRect.height();
                stateFlags.add(ViewportIsScaled);
            }
            else
            {
                size.fWidth = SkScalarRoundToInt(srcRect.width());
                size.fHeight = SkScalarRoundToInt(srcRect.height());
                stateFlags.remove(ViewportIsScaled);
            }
        }
    }

    // Normal case, surface has no viewport
    else
    {
        srcRect.setWH(
            Float32(sizeB.width()) / Float32(current.scale),
            Float32(sizeB.height()) / Float32(current.scale));
        size.fWidth = SkScalarRoundToInt(srcRect.width());
        size.fHeight = SkScalarRoundToInt(srcRect.height());
        stateFlags.remove(ViewportIsCropped);
        stateFlags.remove(ViewportIsScaled);
    }

    if (prevSize != size)
        current.changesToNotify.add(Changes::SizeChanged);

    if (prevSrcRect != srcRect)
        current.changesToNotify.add(Changes::SrcRectChanged);

    return true;
}

void LSurface::LSurfacePrivate::notifyRoleChange() noexcept
{
    surfaceResource->surface()->roleChanged();
}

void LSurface::LSurfacePrivate::destroyCursorOrDNDRole()
{
    auto *surface { surfaceResource->surface() };
    if (surface->dndIcon())
    {
        auto *role { surface->dndIcon() };
        compositor()->onAnticipatedObjectDestruction(role);
        surface->imp()->setMapped(false);
        surface->imp()->setRole(nullptr, true);
        delete role;
    }
    else if (surface->cursorRole())
    {
        auto *role { surface->cursorRole() };
        compositor()->onAnticipatedObjectDestruction(role);
        surface->imp()->setMapped(false);
        surface->imp()->setRole(nullptr, true);
        delete role;
    }
}

bool LSurface::LSurfacePrivate::canHostRole() const noexcept
{
    return role == nullptr && xdgSurface == nullptr;
}

void LSurface::LSurfacePrivate::checkTimelines() noexcept
{
    if (!pending.drmSyncObjSurfaceRes)
    {
        pending.buffer.acquireTimeline = {};
        pending.buffer.releaseTimeline = {};
        return;
    }

    if (pending.buffer.buffer.res() && pending.buffer.attached)
    {
        if (!pending.buffer.acquireTimeline)
        {
            pending.drmSyncObjSurfaceRes->postError(WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_ACQUIRE_POINT, "Buffer set but no acquire timeline point");
            return;
        }

        if (!pending.buffer.releaseTimeline)
        {
            pending.drmSyncObjSurfaceRes->postError(WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_RELEASE_POINT, "Buffer set but no release timeline point");
            return;
        }

        if (pending.buffer.acquireTimeline == pending.buffer.releaseTimeline && pending.buffer.acquirePoint >= pending.buffer.releasePoint)
        {
            pending.drmSyncObjSurfaceRes->postError(WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_CONFLICTING_POINTS, "Acquire and release timeline points are in conflict");
            return;
        }

        const auto waitRet { pending.buffer.acquireTimeline->waitSync(pending.buffer.acquirePoint, DRM_SYNCOBJ_WAIT_FLAGS_WAIT_AVAILABLE) };

        if (waitRet == 0)
        {
            LLog(CZTrace, CZLN, "Locking surface commit: Acquire timeline point not yet materialized");
            acquireTimelineLocks.emplace_back(lock());
            const auto commitId { pending.commitId };
            pending.buffer.acquireTimelineSource = pending.buffer.acquireTimeline->waitAsync(
                pending.buffer.acquirePoint,
                DRM_SYNCOBJ_WAIT_FLAGS_WAIT_AVAILABLE,
                [this, commitId](RDRMTimeline *){
                    for (size_t i = 0; i < acquireTimelineLocks.size(); i++)
                    {
                        if (acquireTimelineLocks[i]->commitId() == commitId)
                        {
                            LLog(CZTrace, CZLN, "Unlocking surface commit: Acquire timeline point materialized");
                            acquireTimelineLocks[i] = std::move(acquireTimelineLocks.back());
                            acquireTimelineLocks.pop_back();
                            return;
                        }
                    }
                });
            return;
        }
        else if (waitRet == -1)
        {
            wl_client_post_no_memory(pending.drmSyncObjSurfaceRes->client()->client());
            return;
        }
        // else already materialized
    }
    else
    {
        if (pending.buffer.acquireTimeline || pending.buffer.releaseTimeline)
        {
            pending.drmSyncObjSurfaceRes->postError(WP_LINUX_DRM_SYNCOBJ_SURFACE_V1_ERROR_NO_BUFFER, "Timeline point set but no buffer");
            return;
        }
    }
}

void LSurface::LSurfacePrivate::clearUncommitted(Uncommitted &pending) noexcept
{
    pending.buffer.acquireTimeline = {};
    pending.buffer.releaseTimeline = {};
    pending.buffer.acquireTimelineSource = {};
    pending.buffer.attached = false;
    pending.damage.clear();
    pending.bufferDamage.clear();
}

void LSurface::LSurfacePrivate::handleCommit() noexcept
{
    CZWeak<LSurface> ref { surfaceResource->surface() };
    pending.buffer.surface = surfaceResource->surface();

    checkTimelines();

    if (!ref)
        return;

    if (ref->role())
        ref->role()->cacheCommit();

    if (!ref)
        return;

    // Apply this and all next unlocked states
    while (!cached.empty() && cached.front()->lockCount == 0)
    {
        auto state { cached.front() };
        cached.pop_front();
        applyCommit(*state);

        // In case of a protocol error
        if (!ref) return;
    }

    if (!cached.empty() || pending.lockCount != 0)
    {
        cached.emplace_back(std::shared_ptr<Uncommitted>(new Uncommitted(pending)));
        clearUncommitted(pending);
        pending.lockCount = 0;
        pending.changesToNotify = 0;
        pending.commitId++;
        return;
    }

    applyCommit(pending);

    if (!ref)
        return;

    clearUncommitted(pending);
    pending.changesToNotify = 0;
    pending.commitId++;
}

void LSurface::LSurfacePrivate::unlockCommit(UInt32 commitId) noexcept
{
    // If still pending, do nothing and wait for a real commit
    if (pending.commitId == commitId)
    {
        assert(pending.lockCount != 0);
        pending.lockCount--;
        return;
    }

    for (auto &state : cached)
    {
        if (state->commitId == commitId)
        {
            assert(state->lockCount != 0);
            state->lockCount--;
            break;
        }
    }

    CZWeak<LSurface> ref { surfaceResource->surface() };

    // Apply this and all next unlocked states
    while (!cached.empty() && cached.front()->lockCount == 0)
    {
        auto state { cached.front() };
        cached.pop_front();
        applyCommit(*state);

        // In case of a protocol error
        if (!ref) return;
    }
}

void LSurface::LSurfacePrivate::applySubsurfacesOrder(Uncommitted &pending) noexcept
{
    // TODO: Add hint to skip this

    auto oldSubsurfacesAbove { std::move(current.subsurfacesAbove) };
    current.subsurfacesAbove = {};
    auto oldSubsurfacesBelow { std::move(current.subsurfacesBelow) };
    current.subsurfacesBelow = {};

    // Copy new subsurfaces and mark ones that changed order

    size_t i { 0 }; // Old index
    for (const auto &subsurface : pending.subsurfacesAbove)
    {
        // Destroyed
        if (!subsurface)
            continue;

        current.subsurfacesAbove.emplace_back(subsurface.get());
        subsurface->m_changedOrder = oldSubsurfacesAbove.size() < i + 1 || oldSubsurfacesAbove[i] != subsurface.get();
        i++;
    }

    i = 0;
    for (const auto &subsurface : pending.subsurfacesBelow)
    {
        // Destroyed
        if (!subsurface)
            continue;

        current.subsurfacesBelow.emplace_back(subsurface.get());
        subsurface->m_changedOrder = oldSubsurfacesBelow.size() < i + 1 || oldSubsurfacesBelow[i] != subsurface.get();
        i++;
    }

    // Notify

    for (i = 0; i < current.subsurfacesAbove.size(); i++)
    {
        current.subsurfacesAbove[i]->m_parentAlreadyCommitted = false;

        if (!current.subsurfacesAbove[i]->m_changedOrder)
            continue;

        current.subsurfacesAbove[i]->m_changedOrder = false;
        if (i == 0)
            current.subsurfacesAbove[i]->placedAbove(surfaceResource->surface());
        else
            current.subsurfacesAbove[i]->placedAbove(current.subsurfacesAbove[i - 1]->surface());
    }

    if (current.subsurfacesBelow.empty())
        return;

    for (i = current.subsurfacesBelow.size(); i > 0; i--)
    {
        const auto I { i - 1};

        current.subsurfacesBelow[I]->m_parentAlreadyCommitted = false;

        if (!current.subsurfacesBelow[I]->m_changedOrder)
            continue;

        current.subsurfacesBelow[I]->m_changedOrder = false;

        if (i == current.subsurfacesBelow.size())
            current.subsurfacesBelow[I]->placedBelow(surfaceResource->surface());
        else
            current.subsurfacesBelow[I]->placedBelow(current.subsurfacesBelow[i]->surface());
    }
}

bool LSurface::LSurfacePrivate::notifyCommitToSubsurfaces() noexcept
{
    CZWeak<LSurface> ref { surfaceResource->surface() };

    // m_parentAlreadyCommitted is reset in applySubsurfacesOrder

retryBelow:
    stateFlags.remove(SubsurfacesListChanged);

    for (auto *sub : current.subsurfacesBelow)
    {
        if (sub->m_parentAlreadyCommitted)
            continue;

        sub->m_parentAlreadyCommitted = true;
        sub->handleParentCommit();

        // Subsurface was destroyed or similar
        if (stateFlags.has(SubsurfacesListChanged))
            goto retryBelow;

        // Protocol error
        if (!ref)
            return false;
    }

retryAbove:
    stateFlags.remove(SubsurfacesListChanged);

    for (auto *sub : current.subsurfacesAbove)
    {
        if (sub->m_parentAlreadyCommitted)
            continue;

        sub->m_parentAlreadyCommitted = true;
        sub->handleParentCommit();

        // Subsurface was destroyed or similar
        if (stateFlags.has(SubsurfacesListChanged))
            goto retryAbove;

        // Protocol error
        if (!ref)
            return false;
    }

    return true;
}

void LSurface::LSurfacePrivate::applyCommit(Uncommitted &pending) noexcept
{
    current.commitId = pending.commitId;
    current.changesToNotify = pending.changesToNotify;
    pending.changesToNotify = 0;

    // If there are current (old) presentation feedback resources
    // then the content for that commit was never presented
    while (!current.presentationFeedbackRes.empty())
    {
        if (current.presentationFeedbackRes.front())
            current.presentationFeedbackRes.front()->discarded();

        current.presentationFeedbackRes.pop_front();
    }

    current.presentationFeedbackRes = std::move(pending.presentationFeedbackRes);
    pending.presentationFeedbackRes = {};

    auto *surface { surfaceResource->surface() };
    auto &changes { current.changesToNotify };
    applySubsurfacesOrder(pending);

    // False on protocol error
    if (!notifyCommitToSubsurfaces())
        return;

    if (pending.buffer.attached)
    {
        if (current.buffer.buffer.res())
        {
            // Release DMA buffers only if a second one has been attached
            if (LDMABuffer::isDMABuffer(current.buffer.buffer.res()) && current.buffer.buffer.res() != pending.buffer.buffer.res())
            {
                current.buffer.release();
                wl_client_flush(wl_resource_get_client(current.buffer.buffer.res()));
            }
        }

        current.buffer = pending.buffer;
    }

    // Add pending frames to the end of current
    if (!pending.frames.resources.empty())
    {
        current.frames.resources.splice(current.frames.resources.end(), pending.frames.resources);
        surface->requestedRepaint();
    }

    /*****************************************
     *********** BUFFER TO TEXTURE ***********
     *****************************************/

    // Turn buffer into RImage and process damage
    if (current.buffer.buffer.res())
    {
        // Returns false on wl_client destroy
        if (!bufferToImage(pending))
        {
            LLog(CZError, CZLN, "Failed to create RImage from client buffer");
            return;
        }
    }

    const SkIRect surfaceSizeRect { SkIRect::MakeSize(surface->size()) };

    /************************************
     *********** INPUT REGION ***********
     ************************************/
    if (surface->receiveInput())
    {
        if (stateFlags.has(InfiniteInput))
        {
            if (changes.has(Changes::SizeChanged | Changes::InputRegionChanged))
            {
                current.inputRegion.setRect(surfaceSizeRect);
                changes.add(Changes::InputRegionChanged);
            }
        }
        else if (changes.has(Changes::SizeChanged | Changes::InputRegionChanged))
        {
            current.inputRegion.op(surfaceSizeRect, pending.inputRegion, SkRegion::Op::kIntersect_Op);
            changes.add(Changes::InputRegionChanged);
        }
    }
    else
    {
        /******************************************
         *********** CLEAR INPUT REGION ***********
         ******************************************/
        current.inputRegion.setEmpty();
        pending.pointerConstraintRegion.reset();
        current.pointerConstraintRegion.setEmpty();
    }

    /******************************************
     *********** POINTER CONSTRAINT ***********
     ******************************************/

    PointerConstraintMode pendingPointerConstraintMode;

    if (pending.lockedPointerRes)        pendingPointerConstraintMode = PointerConstraintMode::Lock;
    else if (pending.confinedPointerRes) pendingPointerConstraintMode = PointerConstraintMode::Confine;
    else                                 pendingPointerConstraintMode = PointerConstraintMode::Free;

    current.confinedPointerRes = pending.confinedPointerRes;
    current.lockedPointerRes   = pending.lockedPointerRes;

    if (current.pointerConstraintMode != pendingPointerConstraintMode)
    {
        current.pointerConstraintMode = pendingPointerConstraintMode;
        changes.add(Changes::PointerConstraintModeChanged | Changes::PointerConstraintRegionChanged | Changes::LockedPointerPosHintChanged);

        if (current.pointerConstraintMode == PointerConstraintMode::Free)
        {
            current.lockedPointerPosHint = pending.lockedPointerPosHint = { -1, -1 };
            pending.pointerConstraintRegion.reset();
            current.pointerConstraintRegion.setEmpty();
        }
    }

    if (surface->pointerConstraintMode() != LSurface::Free)
    {
        if (changes.has(Changes::PointerConstraintRegionChanged | Changes::InputRegionChanged))
        {
            if (pending.pointerConstraintRegion)
                current.pointerConstraintRegion.op(*pending.pointerConstraintRegion, current.inputRegion, SkRegion::Op::kIntersect_Op);
            else
                current.pointerConstraintRegion = current.inputRegion;

            changes.add(Changes::PointerConstraintRegionChanged);
        }

        if (changes.has(Changes::LockedPointerPosHintChanged))
            current.lockedPointerPosHint = pending.lockedPointerPosHint;
    }

    /****************************************
     *********** INVISIBLE REGION ***********
     ****************************************/
    if (stateFlags.has(InfiniteInvisible))
    {
        if (changes.has(Changes::SizeChanged | Changes::InvisibleRegionChanged))
        {
            current.invisibleRegion.setRect(surfaceSizeRect);
            changes.add(Changes::InvisibleRegionChanged);
        }
    }
    else if (changes.has(Changes::SizeChanged | Changes::InvisibleRegionChanged))
    {
        current.invisibleRegion.op(surfaceSizeRect, pending.invisibleRegion, SkRegion::Op::kIntersect_Op);
        changes.add(Changes::InvisibleRegionChanged);
    }

    /************************************
     ********** OPAQUE REGION ***********
     ************************************/
    if (changes.has(Changes::BufferSizeChanged | Changes::SizeChanged | Changes::OpaqueRegionChanged))
    {
        if (current.image && !current.image->formatInfo().alpha)
            current.opaqueRegion.setRect(surfaceSizeRect);
        else
            current.opaqueRegion.op(pending.opaqueRegion, surfaceSizeRect, SkRegion::Op::kIntersect_Op);
    }

    /*******************************************
     ***************** VSYNC *******************
     *******************************************/
    const bool preferVSync { surface->surfaceResource()->tearingControlRes() == nullptr || surface->surfaceResource()->tearingControlRes()->preferVSync() };

    if (stateFlags.has(VSync) != preferVSync)
    {
        changes.add(Changes::VSyncChanged);
        stateFlags.setFlag(VSync, preferVSync);
    }

    /**************************************************
     ***************** CONTENT TYPE *******************
     **************************************************/
    if (pending.contentType != current.contentType)
    {
        changes.add(Changes::ContentTypeChanged);
        current.contentType = pending.contentType;
    }

    CZWeak<LSurface> ref { surface };

    /*******************************************
     *********** NOTIFY COMMIT TO ROLE *********
     *******************************************/
    if (surface->role())
        surface->role()->applyCommit();

    surface->backgroundBlur()->handleCommit(changes.has(Changes::SizeChanged));

    if (!ref)
        return;

    CZCore::Get()->sendEvent(LSurfaceCommitEvent(changes), *ref.get());
    changes.set(Changes::NoChanges);
}

void LSurface::LSurfacePrivate::setLayer(LSurfaceLayer newLayer) noexcept
{
    if (layer == newLayer)
        return;

    auto *surf { surfaceResource->surface() };
    compositor()->imp()->layers[layer].erase(layerLink);
    compositor()->imp()->layers[newLayer].emplace_back(surf);
    layerLink = std::prev(compositor()->imp()->layers[newLayer].end());
    layer = newLayer;

    surf->layerChanged();

    for (auto *sub : surf->subsurfacesAbove())
        sub->surface()->imp()->setLayer(newLayer);

    for (auto *sub : surf->subsurfacesBelow())
        sub->surface()->imp()->setLayer(newLayer);

    if (auto *toplevel = surf->toplevel())
    {
        for (auto *child : toplevel->childToplevels())
            child->surface()->imp()->setLayer(newLayer);

        for (auto *child : toplevel->childPopups())
            child->surface()->imp()->setLayer(newLayer);
    }
    else if (auto *popup = surf->popup())
    {
        for (auto *child : popup->childPopups())
            child->surface()->imp()->setLayer(newLayer);
    }
    else if (auto *layerRole = surf->layerRole())
    {
        for (auto *child : layerRole->childPopups())
            child->surface()->imp()->setLayer(newLayer);
    }
}

void LSurface::LSurfacePrivate::setParent(LSurface *newParent) noexcept
{
    if (parent == newParent)
        return;

    parent = newParent;
    surfaceResource->surface()->parentChanged();

    if (parent)
        setLayer(parent->layer());
    else
        setLayer(LLayerMiddle);
}

std::shared_ptr<LSurfaceLock> LSurface::LSurfacePrivate::lock() noexcept
{
    return std::shared_ptr<LSurfaceLock>(new LSurfaceLock(surfaceResource->surface(), false));
}

bool LSurface::LSurfacePrivate::IsSubsurfaceOf(LSurface *surface, LSurface *parent) noexcept
{
    if (surface == parent)
        return true;

    for (auto &c : parent->imp()->pending.subsurfacesAbove)
        if (c && c->surface() && IsSubsurfaceOf(surface, c->surface()))
            return true;

    for (auto &c : parent->imp()->pending.subsurfacesBelow)
        if (c && c->surface() && IsSubsurfaceOf(surface, c->surface()))
            return true;

    return false;
}
