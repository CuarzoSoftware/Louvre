#include <CZ/Louvre/Protocols/PresentationTime/presentation-time.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/LSinglePixelBuffer.h>
#include <CZ/Louvre/Protocols/FractionalScale/RFractionalScale.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/LDMABuffer.h>
#include <CZ/Louvre/Protocols/Wayland/RCallback.h>
#include <CZ/Louvre/Protocols/Wayland/RSurface.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LTexturePrivate.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LKeyboardPrivate.h>
#include <CZ/Utils/CZRegionUtils.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/LOutputMode.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LTime.h>
#include <CZ/Louvre/LLog.h>
#include <cassert>

void LSurface::LSurfacePrivate::setParent(LSurface *parent)
{
    if (stateFlags.has(Destroyed))
        return;

    if (pendingParent)
    {
        pendingParent->imp()->pendingChildren.erase(pendingParentLink);
        pendingParent = nullptr;
    }

    if (parent == this->parent)
        return;

    LSurface *surface { surfaceResource->surface() };

    if (this->parent)
        this->parent->imp()->children.erase(parentLink);

    this->parent = parent;

    if (parent == nullptr)
        return;

    const bool isSubsurfaceOrToplevel {
        surface->subsurface() != nullptr || surface->toplevel() != nullptr
    };

    using OP = LCompositor::LCompositorPrivate::InsertOptions;

    if (isSubsurfaceOrToplevel)
    {
        if (parent->layer() != surface->layer())
            setLayer(parent->layer());

        if (parent->children().empty())
            compositor()->imp()->insertSurfaceAfter(parent, surface, OP::UpdateSurfaces | OP::UpdateLayers);
        else
        {
            bool inserted { false };

            for (auto child = parent->children().rbegin(); child != parent->children().rend(); child++)
            {
                if ((*child)->layer() == surface->layer())
                {
                    inserted = true;
                    compositor()->imp()->insertSurfaceAfter(*child, surface, OP::UpdateSurfaces | OP::UpdateLayers);
                    break;
                }
            }

            if (!inserted)
                compositor()->imp()->insertSurfaceAfter(parent, surface, OP::UpdateSurfaces | OP::UpdateLayers);
        }
    }

    parent->imp()->children.push_back(surface);
    surface->imp()->parentLink = std::prev(parent->imp()->children.end());
    surface->parentChanged();

    if (surface->role())
        surface->role()->handleParentChange();
}

void LSurface::LSurfacePrivate::removeChild(LSurface *child)
{
    if (stateFlags.has(Destroyed))
        return;

    children.erase(child->imp()->parentLink);
    child->imp()->parent = nullptr;
    child->parentChanged();
}

void LSurface::LSurfacePrivate::setMapped(bool state)
{
    if (stateFlags.has(Destroyed))
        return;

    LSurface *surface { surfaceResource->surface() };

    if (stateFlags.has(Mapped) != state)
    {
        stateFlags.setFlag(Mapped, state);

        if (!state)
        {
            while (!frameCallbacks.empty())
            {
                frameCallbacks.front()->done(LTime::ms());
                frameCallbacks.front()->destroy();
            }
        }

        surface->mappingChanged();

        /* We create a copy of the childrens list
         * because a child could be removed/destroyed
         * when handleParentMappingChange() is called */

        std::list<CZWeak<LSurface>> childrenTmp;

        for (LSurface *s : children)
            childrenTmp.emplace_back(s);

        while (!childrenTmp.empty())
        {
            if (!childrenTmp.front().get())
                goto skip;

            if (childrenTmp.front()->role())
                childrenTmp.front()->role()->handleParentMappingChange();

            skip:
            childrenTmp.pop_front();
        }
    }
}

void LSurface::LSurfacePrivate::setRole(LBaseSurfaceRole *role) noexcept
{
    assert(role != this->role && "Setting the same role twice");
    assert(!stateFlags.has(UnnotifiedRoleChange) && "Setting new role without notifying previous change");
    stateFlags.add(UnnotifiedRoleChange);

    prevRole = this->role;

    if (role)
    {
        assert(this->role == nullptr && "The previous role was not unset");
        this->role = role;
    }
    else
    {
        assert(this->role != nullptr && "The was no previous role");
        this->role = nullptr;
    }
}

void LSurface::LSurfacePrivate::applyPendingChildren()
{
    using OP = LCompositor::LCompositorPrivate::InsertOptions;

    if (pendingChildren.empty())
        return;

    LSurface *surface { surfaceResource->surface() };

    while (!pendingChildren.empty())
    {
        LSurface *child { pendingChildren.front() };
        pendingChildren.pop_front();

        if (child->imp()->pendingParent != surface)
            continue;

        // If the child already had a parent
        if (child->imp()->parent)
            child->imp()->parent->imp()->children.erase(child->imp()->parentLink);

        const bool isSubsurfaceOrToplevel {
            child->subsurface() != nullptr  ||
            child->toplevel() != nullptr };

        if (isSubsurfaceOrToplevel)
        {
            if (child->layer() != surface->layer())
                child->imp()->setLayer(surface->layer());

            if (surface->children().empty())
                compositor()->imp()->insertSurfaceAfter(surface, child, OP::UpdateSurfaces | OP::UpdateLayers);
            else
            {
                bool inserted { false };

                for (auto c = surface->children().rbegin(); c != surface->children().rend(); c++)
                {
                    if ((*c)->layer() == child->layer())
                    {
                        inserted = true;
                        compositor()->imp()->insertSurfaceAfter(*c, child, OP::UpdateSurfaces | OP::UpdateLayers);
                        break;
                    }
                }

                if (!inserted)
                    compositor()->imp()->insertSurfaceAfter(surface, child, OP::UpdateSurfaces | OP::UpdateLayers);
            }
        }

        children.push_back(child);
        child->imp()->pendingParent = nullptr;

        child->imp()->parent = surface;
        child->imp()->parentLink = std::prev(children.end());
        child->parentChanged();

        if (child->role())
            child->role()->handleParentChange();
    }

    compositor()->imp()->surfaceRaiseAllowedCounter++;
    surface->orderChanged();
    compositor()->imp()->surfaceRaiseAllowedCounter--;
}

bool LSurface::LSurfacePrivate::bufferToTexture() noexcept
{
    // Only for wl_drm case
    GLint format;

    // Size of the current buffer without transform
    Int32 widthB, heightB;

    /***************************************
     *********** BUFFER TRANSFOM ***********
     ***************************************/

    if (current.transform != pending.transform)
    {
        current.transform = pending.transform;
        changesToNotify.add(BufferTransformChanged);
    }

    /***********************************
     *********** BUFFER SCALE ***********
     ***********************************/

    if (current.bufferScale != pending.bufferScale)
    {
        current.bufferScale = pending.bufferScale;
        changesToNotify.add(BufferScaleChanged);
    }

    if (current.bufferRes)
    {
        // SHM
        if (wl_shm_buffer_get(current.bufferRes))
        {
            if (!stateFlags.has(BufferReleased))
            {
                wl_buffer_send_release(current.bufferRes);
                stateFlags.add(BufferReleased);
            }

            if (texture && texture != textureBackup && texture->m_pendingDelete)
                delete texture;

            texture = textureBackup;

            wl_shm_buffer *shm_buffer = wl_shm_buffer_get(current.bufferRes);
            wl_shm_buffer_begin_access(shm_buffer);
            UInt8 *pixels = (UInt8*)wl_shm_buffer_get_data(shm_buffer);
            UInt32 format =  LTexture::waylandFormatToDRM(wl_shm_buffer_get_format(shm_buffer));
            Int32 stride = wl_shm_buffer_get_stride(shm_buffer);
            widthB = wl_shm_buffer_get_width(shm_buffer);
            heightB = wl_shm_buffer_get_height(shm_buffer);

            if (!updateDimensions(widthB, heightB))
                return false;

            if (!texture->initialized() || changesToNotify.has(SizeChanged | SourceRectChanged | BufferSizeChanged | BufferTransformChanged | BufferScaleChanged))
            {
                currentDamageB.setRect(SkIRect::MakeSize(sizeB));
                currentDamage.setRect(SkIRect::MakeSize(size));
                texture->setDataFromMainMemory(SkISize(widthB, heightB), stride, format, pixels);
            }
            else if (!pendingDamageB.empty() || !pendingDamage.empty())
            {
                simplifyDamage(pendingDamageB);
                simplifyDamage(pendingDamage);

                SkRegion onlyPending;

                if (stateFlags.has(ViewportIsScaled | ViewportIsCropped))
                {
                    Float32 xInvScale = (Float32(current.bufferScale) * srcRect.width())/Float32(size.width());
                    Float32 yInvScale = (Float32(current.bufferScale) * srcRect.height())/Float32(size.height());

                    Int32 xOffset = roundf(srcRect.x() * Float32(current.bufferScale)) - 2;
                    Int32 yOffset = roundf(srcRect.y() * Float32(current.bufferScale)) - 2;

                    while (!pendingDamage.empty())
                    {
                        SkIRect &r = pendingDamage.back();
                        onlyPending.op(
                            SkIRect::MakeXYWH(
                                (r.x() * xInvScale + xOffset),
                                (r.y() * yInvScale + yOffset),
                                (r.width() * xInvScale + 4 ),
                                (r.height() * yInvScale + 4 )),
                            SkRegion::Op::kUnion_Op);
                        pendingDamage.pop_back();
                    }

                    while (!pendingDamageB.empty())
                    {
                        onlyPending.op(pendingDamageB.back().makeOutset(1, 1), SkRegion::Op::kUnion_Op);
                        pendingDamageB.pop_back();
                    }

                    CZRegionUtils::ApplyTransform(onlyPending, sizeB, current.transform);

                    if (!onlyPending.isEmpty() && texture->writeBegin())
                    {
                        const UInt32 pixelSize { LTexture::formatBytesPerPixel(format) };

                        SkRegion::Iterator it(onlyPending);

                        while (!it.done())
                        {
                            texture->writeUpdate(it.rect(),
                                                 stride,
                                                 &pixels[it.rect().x()*pixelSize + it.rect().y()*stride]);
                            it.next();
                        }

                        texture->writeEnd();
                    }

                    CZRegionUtils::ApplyTransform(onlyPending, sizeB, CZ::RequiredTransform(current.transform, CZTransform::Normal));
                    currentDamageB.op(onlyPending, SkRegion::Op::kUnion_Op);
                    currentDamage = currentDamageB;
                    currentDamage.translate(-xOffset - 2, -yOffset - 2);
                    CZRegionUtils::Scale(currentDamage, 1.f/xInvScale, 1.f/yInvScale);
                }
                else
                {
                    while (!pendingDamage.empty())
                    {
                        SkIRect &r = pendingDamage.back();
                        onlyPending.op(
                            SkIRect::MakeXYWH(
                                (r.x() - 2)*current.bufferScale,
                                (r.y() - 2)*current.bufferScale,
                                (r.width() + 4 )*current.bufferScale,
                                (r.height() + 4 )*current.bufferScale),
                            SkRegion::Op::kUnion_Op);
                        pendingDamage.pop_back();
                    }

                    while (!pendingDamageB.empty())
                    {
                        onlyPending.op(pendingDamageB.back().makeOutset(2, 2), SkRegion::Op::kUnion_Op);
                        pendingDamageB.pop_back();
                    }

                    onlyPending.op(SkIRect::MakeSize(sizeB), SkRegion::kIntersect_Op);
                    currentDamageB.op(onlyPending, SkRegion::kUnion_Op);
                    CZRegionUtils::ApplyTransform(onlyPending, sizeB, current.transform);

                    if (!onlyPending.isEmpty() && texture->writeBegin())
                    {
                        const UInt32 pixelSize { LTexture::formatBytesPerPixel(format) };
                        SkRegion::Iterator it(onlyPending);

                        while (!it.done())
                        {
                            texture->writeUpdate(it.rect(),
                                                 stride,
                                                 &pixels[it.rect().x()*pixelSize + it.rect().y()*stride]);
                            it.next();
                        }

                        texture->writeEnd();
                    }

                    CZRegionUtils::Scale(currentDamageB, currentDamage, 1.f/Float32(current.bufferScale));
                }
            }
            else
            {
                wl_shm_buffer_end_access(shm_buffer);
                wl_client_flush(wl_resource_get_client(current.bufferRes));
                return true;
            }

            wl_shm_buffer_end_access(shm_buffer);
            wl_client_flush(wl_resource_get_client(current.bufferRes));
        }

        // WL_DRM
        else if (compositor()->imp()->WL_bind_wayland_display && compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.bufferRes, EGL_TEXTURE_FORMAT, &format))
        {
            /* Unlike SHM buffers, the current buffer is released after
             * a different buffer is commited */

            if (texture && texture != textureBackup && texture->m_pendingDelete)
                delete texture;

            texture = textureBackup;
            compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.bufferRes, EGL_WIDTH, &widthB);
            compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.bufferRes, EGL_HEIGHT, &heightB);
            if (!updateDimensions(widthB, heightB))
                return false;
            updateDamage();
            texture->setDataFromWaylandDRM(current.bufferRes);
        }

        // DMA-Buf
        else if (LDMABuffer::isDMABuffer(current.bufferRes))
        {
            /* Unlike SHM buffers, the current buffer is released after
             * a different buffer is commited */

            LDMABuffer *dmaBuffer = (LDMABuffer*)wl_resource_get_user_data(current.bufferRes);
            widthB = dmaBuffer->planes()->width;
            heightB = dmaBuffer->planes()->height;

            if (!updateDimensions(widthB, heightB))
                return false;

            if (!dmaBuffer->texture())
            {
                dmaBuffer->m_texture = new LTexture(true);
                dmaBuffer->texture()->setDataFromDMA(*dmaBuffer->planes());
            }

            updateDamage();

            if (texture && texture != textureBackup && texture->m_pendingDelete)
                delete texture;

            texture = dmaBuffer->texture();
        }
        // Single pixel buffer
        else if (LSinglePixelBuffer::isSinglePixelBuffer(current.bufferRes))
        {
            if (!stateFlags.has(BufferReleased))
            {
                wl_buffer_send_release(current.bufferRes);
                stateFlags.add(BufferReleased);
            }

            if (texture && texture != textureBackup && texture->m_pendingDelete)
                delete texture;

            texture = textureBackup;
            widthB = heightB = 1;

            if (!updateDimensions(widthB, heightB))
                return false;

            LSinglePixelBuffer &singlePixelBuffer { *static_cast<LSinglePixelBuffer*>(wl_resource_get_user_data(current.bufferRes)) };

            UInt8 buffer[4]
            {
                static_cast<UInt8>(
                    (static_cast<UInt64>(singlePixelBuffer.pixel().b) * static_cast<UInt64>(255))
                    /static_cast<UInt64>(std::numeric_limits<UInt32>::max())),
                static_cast<UInt8>(
                    (static_cast<UInt64>(singlePixelBuffer.pixel().g) * static_cast<UInt64>(255))
                    /static_cast<UInt64>(std::numeric_limits<UInt32>::max())),
                static_cast<UInt8>(
                    (static_cast<UInt64>(singlePixelBuffer.pixel().r) * static_cast<UInt64>(255))
                    /static_cast<UInt64>(std::numeric_limits<UInt32>::max())),
                static_cast<UInt8>(
                    (static_cast<UInt64>(singlePixelBuffer.pixel().a) * static_cast<UInt64>(255))
                    /static_cast<UInt64>(std::numeric_limits<UInt32>::max())),
            };

            texture->setDataFromMainMemory(SkISize(1, 1), 4, DRM_FORMAT_ARGB8888, buffer);
            updateDamage();
        }
        else
        {
            LLog::error("[LSurfacePrivate::bufferToTexture] Unknown buffer type. Killing client.");
            surfaceResource->postError(0, "Unknown buffer type.");
            return false;
        }
    }
    else
    {
        if (!texture)
            texture = textureBackup;

        widthB = texture->sizeB().width();
        heightB = texture->sizeB().height();

        if (!updateDimensions(widthB, heightB))
            return false;

        updateDamage();
    }

    texture->m_surface.reset(surfaceResource->surface());
    pendingDamageB.clear();
    pendingDamage.clear();
    damageId = LTime::nextSerial();
    stateFlags.add(Damaged);
    return true;
}

void LSurface::LSurfacePrivate::sendPresentationFeedback(LOutput *output) noexcept
{
    if (presentationFeedbackResources.empty())
        return;

    for (std::size_t i = 0; i < presentationFeedbackResources.size();)
    {
        const UInt32 scanout =
            surfaceResource->surface() == output->imp()->scanout[0].surface ||
            surfaceResource->surface() == output->imp()->scanout[1].surface ?
            WP_PRESENTATION_FEEDBACK_KIND_ZERO_COPY : 0;

        auto *feedback { presentationFeedbackResources[i] };

        if (!scanout && (feedback->m_commitId == -2 || (feedback->m_outputSet && !feedback->m_output)))
        {
            feedback->discarded();
            feedback->m_surface.reset();
            presentationFeedbackResources[i] = std::move(presentationFeedbackResources.back());
            presentationFeedbackResources.pop_back();
            wl_resource_destroy(feedback->resource());
            continue;
        }
        else if (scanout || feedback->m_output == output)
        {
            for (Wayland::GOutput *gOutput : surfaceResource->client()->outputGlobals())
                if (gOutput->output() == output)
                    feedback->syncOutput(gOutput);

            feedback->presented(output->imp()->presentationTime.time.tv_sec >> 32,
                             output->imp()->presentationTime.time.tv_sec & 0xffffffff,
                             (UInt32)output->imp()->presentationTime.time.tv_nsec,
                             output->imp()->presentationTime.period,
                             output->imp()->presentationTime.frame >> 32,
                             output->imp()->presentationTime.frame & 0xffffffff,
                             output->imp()->presentationTime.flags | scanout);
            feedback->m_surface.reset();
            presentationFeedbackResources[i] = std::move(presentationFeedbackResources.back());
            presentationFeedbackResources.pop_back();
            wl_resource_destroy(feedback->resource());
            continue;
        }

        i++;
    }
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

void LSurface::LSurfacePrivate::setPendingParent(LSurface *pendParent) noexcept
{
    if (pendingParent)
        pendingParent->imp()->pendingChildren.erase(pendingParentLink);

    pendingParent = pendParent;

    if (pendingParent)
    {
        pendingParent->imp()->pendingChildren.push_back(surfaceResource->surface());
        pendingParentLink = std::prev(pendingParent->imp()->pendingChildren.end());
    }
}

bool LSurface::LSurfacePrivate::isInChildrenOrPendingChildren(LSurface *child) noexcept
{
    if (child == surfaceResource->surface())
        return true;

    if (child->imp()->pendingParent)
    {
        if (isInChildrenOrPendingChildren(child->imp()->pendingParent))
            return true;
    }

    for (LSurface *s : children)
    {
        if (s == child)
            return true;

        if (s->imp()->isInChildrenOrPendingChildren(child))
            return true;
    }

    return false;
}

bool LSurface::LSurfacePrivate::hasBufferOrPendingBuffer() noexcept
{
    return current.hasBuffer || pending.hasBuffer;
}

void LSurface::LSurfacePrivate::setKeyboardGrabToParent()
{
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
}

void LSurface::LSurfacePrivate::updateDamage() noexcept
{
    if (!texture->initialized() || changesToNotify.has(SizeChanged | SourceRectChanged | BufferSizeChanged | BufferTransformChanged | BufferScaleChanged))
    {
        currentDamageB.setRect(SkIRect::MakeSize(sizeB));
        currentDamage.setRect(SkIRect::MakeSize(size));
    }
    else if (!pendingDamageB.empty() || !pendingDamage.empty())
    {
        simplifyDamage(pendingDamageB);
        simplifyDamage(pendingDamage);

        if (stateFlags.has(ViewportIsScaled | ViewportIsCropped))
        {
            Float32 xInvScale = (Float32(current.bufferScale) * srcRect.width())/Float32(size.width());
            Float32 yInvScale = (Float32(current.bufferScale) * srcRect.height())/Float32(size.height());

            Int32 xOffset = roundf(srcRect.x() * Float32(current.bufferScale)) - 2;
            Int32 yOffset = roundf(srcRect.y() * Float32(current.bufferScale)) - 2;

            while (!pendingDamage.empty())
            {
                SkIRect &r = pendingDamage.back();
                currentDamageB.op(
                    SkIRect::MakeXYWH(
                        (r.x() * xInvScale + xOffset),
                        (r.y() * yInvScale + yOffset),
                        (r.width() * xInvScale + 4 ),
                        (r.height() * yInvScale + 4 )),
                    SkRegion::Op::kUnion_Op);
                pendingDamage.pop_back();
            }

            while (!pendingDamageB.empty())
            {
                SkIRect &r = pendingDamageB.back();
                currentDamageB.op(r.makeOutset(1, 1), SkRegion::Op::kUnion_Op);
                pendingDamageB.pop_back();
            }

            currentDamageB.op(SkIRect::MakeSize(sizeB), SkRegion::Op::kIntersect_Op);
            currentDamage = currentDamageB;
            currentDamage.translate(-xOffset - 2, -yOffset - 2);
            CZRegionUtils::Scale(currentDamage, 1.f/xInvScale, 1.f/yInvScale);
        }
        else
        {
            while (!pendingDamage.empty())
            {
                SkIRect &r = pendingDamage.back();
                currentDamageB.op(
                    SkIRect::MakeXYWH(
                        (r.x() - 1 )*current.bufferScale,
                        (r.y() - 1 )*current.bufferScale,
                        (r.width() + 2 )*current.bufferScale,
                        (r.height() + 2 )*current.bufferScale),
                    SkRegion::Op::kUnion_Op);
                pendingDamage.pop_back();
            }

            while (!pendingDamageB.empty())
            {
                SkIRect &r = pendingDamageB.back();
                currentDamageB.op(r.makeOutset(1, 1), SkRegion::Op::kUnion_Op);
                pendingDamageB.pop_back();
            }

            currentDamageB.op(SkIRect::MakeSize(sizeB), SkRegion::Op::kIntersect_Op);
            CZRegionUtils::Scale(currentDamageB, currentDamage, 1.f/Float32(current.bufferScale));
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
        changesToNotify.add(BufferSizeChanged);

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

            if (roundf((srcRect.x() + srcRect.width()) * Float32(current.bufferScale)) > sizeB.width() || roundf((srcRect.y() + srcRect.height()) * Float32(current.bufferScale)) > sizeB.height())
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
                Float32(sizeB.width()) / Float32(current.bufferScale),
                Float32(sizeB.height()) / Float32(current.bufferScale));
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
            Float32(sizeB.width()) / Float32(current.bufferScale),
            Float32(sizeB.height()) / Float32(current.bufferScale));
        size.fWidth = SkScalarRoundToInt(srcRect.width());
        size.fHeight = SkScalarRoundToInt(srcRect.height());
        stateFlags.remove(ViewportIsCropped);
        stateFlags.remove(ViewportIsScaled);
    }

    if (prevSize != size)
        changesToNotify.add(SizeChanged);

    if (prevSrcRect != srcRect)
        changesToNotify.add(SourceRectChanged);

    return true;
}

void LSurface::LSurfacePrivate::simplifyDamage(std::vector<SkIRect> &vec) noexcept
{
    if (vec.size() >= LOUVRE_MAX_DAMAGE_RECTS)
    {
        SkIRect extents { vec.back() };
        vec.pop_back();

        while (!vec.empty())
        {
            if (vec.back().fLeft < extents.fLeft)
                extents.fLeft = vec.back().fLeft;

            if (vec.back().fTop < extents.fTop)
                extents.fTop = vec.back().fTop;

            if (vec.back().fRight > extents.fRight)
                extents.fRight = vec.back().fRight;

            if (vec.back().fBottom > extents.fBottom)
                extents.fBottom = vec.back().fBottom;

            vec.pop_back();
        }

        vec.emplace_back(extents);
    }
}

void LSurface::LSurfacePrivate::setLayer(LSurfaceLayer newLayer)
{
    const bool layerChanged { layer != newLayer };

    compositor()->imp()->layers[layer].erase(layerLink);
    layer = newLayer;
    compositor()->imp()->layers[layer].emplace_back(surfaceResource->surface());
    layerLink = std::prev(compositor()->imp()->layers[layer].end());
    LSurface *prev { prevSurfaceInLayers() };

    if (layerChanged)
        surfaceResource->surface()->layerChanged();

    compositor()->imp()->insertSurfaceAfter(prev, surfaceResource->surface(), LCompositor::LCompositorPrivate::UpdateSurfaces);

    for (LSurface *child : pendingChildren)
        if (child->subsurface() || child->toplevel())
            child->imp()->setLayer(layer);

    for (LSurface *child : children)
        if (child->subsurface() || child->toplevel())
            child->imp()->setLayer(layer);
}

LSurface *LSurface::LSurfacePrivate::prevSurfaceInLayers() noexcept
{
    if (surfaceResource->surface() != compositor()->layer(layer).front())
        return *std::prev(layerLink);

    Int32 prevlayer { layer - 1 };

    while (prevlayer >= 0)
    {
        if (compositor()->layer((LSurfaceLayer)prevlayer).empty())
        {
            prevlayer--;
            continue;
        }

        return compositor()->layer((LSurfaceLayer)prevlayer).back();
    }

    return nullptr;
}

void LSurface::LSurfacePrivate::notifyRoleChange()
{
    assert(stateFlags.has(UnnotifiedRoleChange) && "There is no role change to notify");
    assert(!surfaceResource->surface()->mapped() && "The surface can't be mapped while roleChanged() is called");
    assert(prevRole != role && "The new and previous role are the same");

    stateFlags.remove(UnnotifiedRoleChange);
    surfaceResource->surface()->roleChanged(prevRole.get());
}

void LSurface::LSurfacePrivate::destroyCursorOrDNDRole()
{
    auto *surface { surfaceResource->surface() };
    if (surface->dndIcon())
    {
        compositor()->onAnticipatedObjectDestruction(surface->dndIcon());
        surface->imp()->setMapped(false);
        surface->imp()->setRole(nullptr);
        surface->imp()->notifyRoleChange();
        delete surface->dndIcon();
    }
    else if (surface->cursorRole())
    {
        compositor()->onAnticipatedObjectDestruction(surface->cursorRole());
        surface->imp()->setMapped(false);
        surface->imp()->setRole(nullptr);
        surface->imp()->notifyRoleChange();
        delete surface->cursorRole();
    }
}
