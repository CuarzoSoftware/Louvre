#include <protocols/PresentationTime/presentation-time.h>
#include <protocols/PresentationTime/RPresentationFeedback.h>
#include <protocols/SinglePixelBuffer/LSinglePixelBuffer.h>
#include <protocols/FractionalScale/RFractionalScale.h>
#include <protocols/LinuxDMABuf/LDMABuffer.h>
#include <protocols/Wayland/RCallback.h>
#include <protocols/Wayland/RSurface.h>
#include <protocols/Wayland/GOutput.h>
#include <private/LCompositorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LDNDIconRole.h>
#include <LCursorRole.h>
#include <LOutputMode.h>
#include <LClient.h>
#include <LTime.h>
#include <LLog.h>
#include <cassert>

void LSurface::LSurfacePrivate::setParent(LSurface *parent)
{
    if (stateFlags.check(Destroyed))
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
    if (stateFlags.check(Destroyed))
        return;

    children.erase(child->imp()->parentLink);
    child->imp()->parent = nullptr;
    child->parentChanged();
}

void LSurface::LSurfacePrivate::setMapped(bool state)
{
    if (stateFlags.check(Destroyed))
        return;

    LSurface *surface { surfaceResource->surface() };

    if (stateFlags.check(Mapped) != state)
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

        std::list<LWeak<LSurface>> childrenTmp;

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
    assert(!stateFlags.check(UnnotifiedRoleChange) && "Setting new role without notifying previous change");
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
            if (!stateFlags.check(BufferReleased))
            {
                wl_buffer_send_release(current.bufferRes);
                stateFlags.add(BufferReleased);
            }

            if (texture && texture != textureBackup && texture->m_pendingDelete)
                delete texture;

            texture = textureBackup;

            wl_shm_buffer *shm_buffer = wl_shm_buffer_get(current.bufferRes);
            wl_shm_buffer_begin_access(shm_buffer);
            UChar8 *pixels = (UChar8*)wl_shm_buffer_get_data(shm_buffer);
            UInt32 format =  LTexture::waylandFormatToDRM(wl_shm_buffer_get_format(shm_buffer));
            Int32 stride = wl_shm_buffer_get_stride(shm_buffer);
            widthB = wl_shm_buffer_get_width(shm_buffer);
            heightB = wl_shm_buffer_get_height(shm_buffer);

            if (!updateDimensions(widthB, heightB))
                return false;

            if (!texture->initialized() || changesToNotify.check(SizeChanged | SourceRectChanged | BufferSizeChanged | BufferTransformChanged | BufferScaleChanged))
            {
                currentDamageB.clear();
                currentDamageB.addRect(LRect(0, sizeB));
                currentDamage.clear();
                currentDamage.addRect(LRect(0, size));
                texture->setDataFromMainMemory(LSize(widthB, heightB), stride, format, pixels);
            }
            else if (!pendingDamageB.empty() || !pendingDamage.empty())
            {
                simplifyDamage(pendingDamageB);
                simplifyDamage(pendingDamage);

                LRegion onlyPending;

                if (stateFlags.check(ViewportIsScaled | ViewportIsCropped))
                {
                    Float32 xInvScale = (Float32(current.bufferScale) * srcRect.w())/Float32(size.w());
                    Float32 yInvScale = (Float32(current.bufferScale) * srcRect.h())/Float32(size.h());

                    Int32 xOffset = roundf(srcRect.x() * Float32(current.bufferScale)) - 2;
                    Int32 yOffset = roundf(srcRect.y() * Float32(current.bufferScale)) - 2;

                    while (!pendingDamage.empty())
                    {
                        LRect &r = pendingDamage.back();
                        onlyPending.addRect((r.x() * xInvScale + xOffset),
                                            (r.y() * yInvScale + yOffset),
                                            (r.w() * xInvScale + 4 ),
                                            (r.h() * yInvScale + 4 ));
                        pendingDamage.pop_back();
                    }

                    while (!pendingDamageB.empty())
                    {
                        LRect &r = pendingDamageB.back();
                        onlyPending.addRect(
                            r.x() - 1,
                            r.y() - 1,
                            r.w() + 2,
                            r.h() + 2);
                        pendingDamageB.pop_back();
                    }

                    onlyPending.transform(sizeB, current.transform);

                    Int32 n;
                    const LBox *boxes = onlyPending.boxes(&n);

                    if (n > 0 && texture->writeBegin())
                    {
                        const UInt32 pixelSize { LTexture::formatBytesPerPixel(format) };
                        LRect rect;
                        for (Int32 i = 0; i < n; i++)
                        {
                            rect.setX(boxes->x1);
                            rect.setY(boxes->y1);
                            rect.setW(boxes->x2 - boxes->x1);
                            rect.setH(boxes->y2 - boxes->y1);
                            texture->writeUpdate(rect,
                                                stride,
                                                &pixels[rect.x()*pixelSize + rect.y()*stride]);

                            boxes++;
                        }
                        texture->writeEnd();
                    }

                    onlyPending.transform(sizeB, Louvre::requiredTransform(current.transform, LTransform::Normal));
                    currentDamageB.addRegion(onlyPending);
                    currentDamage = currentDamageB;
                    currentDamage.offset(-xOffset - 2, -yOffset - 2);
                    currentDamage.multiply(1.f/xInvScale, 1.f/yInvScale);
                }
                else
                {
                    while (!pendingDamage.empty())
                    {
                        LRect &r = pendingDamage.back();
                        onlyPending.addRect((r.x() - 2)*current.bufferScale,
                                            (r.y() - 2)*current.bufferScale,
                                            (r.w() + 4 )*current.bufferScale,
                                            (r.h() + 4 )*current.bufferScale);
                        pendingDamage.pop_back();
                    }

                    while (!pendingDamageB.empty())
                    {
                        LRect &r = pendingDamageB.back();
                        onlyPending.addRect(
                            r.x() - 2,
                            r.y() - 2,
                            r.w() + 4,
                            r.h() + 4);
                        pendingDamageB.pop_back();
                    }

                    onlyPending.clip(LRect(0, sizeB));
                    currentDamageB.addRegion(onlyPending);
                    onlyPending.transform(sizeB, current.transform);

                    Int32 n;
                    const LBox *boxes = onlyPending.boxes(&n);

                    if (n > 0 && texture->writeBegin())
                    {
                        const UInt32 pixelSize { LTexture::formatBytesPerPixel(format) };
                        LRect rect;
                        for (Int32 i = 0; i < n; i++)
                        {
                            rect.setX(boxes->x1);
                            rect.setY(boxes->y1);
                            rect.setW(boxes->x2 - boxes->x1);
                            rect.setH(boxes->y2 - boxes->y1);
                            texture->writeUpdate(rect,
                                                stride,
                                                &pixels[rect.x()*pixelSize + rect.y()*stride]);

                            boxes++;
                        }
                        texture->writeEnd();
                    }

                    LRegion::multiply(&currentDamage, &currentDamageB, 1.f/Float32(current.bufferScale));
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
            if (!stateFlags.check(BufferReleased))
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

            texture->setDataFromMainMemory(LSize(1, 1), 4, DRM_FORMAT_ARGB8888, buffer);
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

        widthB = texture->sizeB().w();
        heightB = texture->sizeB().h();

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
    LTransform transform { LTransform::Normal };

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
    if (!texture->initialized() || changesToNotify.check(SizeChanged | SourceRectChanged | BufferSizeChanged | BufferTransformChanged | BufferScaleChanged))
    {
        currentDamageB.clear();
        currentDamageB.addRect(LRect(0, sizeB));
        currentDamage.clear();
        currentDamage.addRect(LRect(0, size));
    }
    else if (!pendingDamageB.empty() || !pendingDamage.empty())
    {
        simplifyDamage(pendingDamageB);
        simplifyDamage(pendingDamage);

        if (stateFlags.check(ViewportIsScaled | ViewportIsCropped))
        {
            Float32 xInvScale = (Float32(current.bufferScale) * srcRect.w())/Float32(size.w());
            Float32 yInvScale = (Float32(current.bufferScale) * srcRect.h())/Float32(size.h());

            Int32 xOffset = roundf(srcRect.x() * Float32(current.bufferScale)) - 2;
            Int32 yOffset = roundf(srcRect.y() * Float32(current.bufferScale)) - 2;

            while (!pendingDamage.empty())
            {
                LRect &r = pendingDamage.back();
                currentDamageB.addRect((r.x() * xInvScale + xOffset),
                                       (r.y() * yInvScale + yOffset),
                                       (r.w() * xInvScale + 4 ),
                                       (r.h() * yInvScale + 4 ));
                pendingDamage.pop_back();
            }

            while (!pendingDamageB.empty())
            {
                LRect &r = pendingDamageB.back();
                currentDamageB.addRect(
                    r.x() - 1,
                    r.y() - 1,
                    r.w() + 2,
                    r.h() + 2);
                pendingDamageB.pop_back();
            }

            currentDamageB.clip(LRect(0, sizeB));
            currentDamage = currentDamageB;
            currentDamage.offset(-xOffset - 2, -yOffset - 2);
            currentDamage.multiply(1.f/xInvScale, 1.f/yInvScale);
        }
        else
        {
            while (!pendingDamage.empty())
            {
                LRect &r = pendingDamage.back();
                currentDamageB.addRect((r.x() - 1 )*current.bufferScale,
                                       (r.y() - 1 )*current.bufferScale,
                                       (r.w() + 2 )*current.bufferScale,
                                       (r.h() + 2 )*current.bufferScale);
                pendingDamage.pop_back();
            }

            while (!pendingDamageB.empty())
            {
                LRect &r = pendingDamageB.back();
                currentDamageB.addRect(
                    r.x() - 1,
                    r.y() - 1,
                    r.w() + 2,
                    r.h() + 2);
                pendingDamageB.pop_back();
            }

            currentDamageB.clip(LRect(0, sizeB));
            LRegion::multiply(&currentDamage, &currentDamageB, 1.f/Float32(current.bufferScale));
        }
    }
}

bool LSurface::LSurfacePrivate::updateDimensions(Int32 widthB, Int32 heightB) noexcept
{
    const LSize prevSizeB { sizeB };
    const LSize prevSize { size };
    const LRectF prevSrcRect { srcRect };

    if (Louvre::is90Transform(current.transform))
    {
        sizeB.setW(heightB);
        sizeB.setH(widthB);
    }
    else
    {
        sizeB.setW(widthB);
        sizeB.setH(heightB);
    }

    if (prevSizeB != sizeB)
        changesToNotify.add(BufferSizeChanged);

    if (surfaceResource->viewportRes())
    {
        bool usingViewportSrc { false };

        // Using the viewport source rect
        if (surfaceResource->viewportRes()->srcRect().x() != -1.f ||
            surfaceResource->viewportRes()->srcRect().y() != -1.f ||
            surfaceResource->viewportRes()->srcRect().w() != -1.f ||
            surfaceResource->viewportRes()->srcRect().h() != -1.f)
        {
            usingViewportSrc = true;

            srcRect = surfaceResource->viewportRes()->srcRect();

            if (srcRect.x() < 0.f || srcRect.y() < 0.f || srcRect.w() <= 0.f || srcRect.h() <= 0.f)
            {
                surfaceResource->viewportRes()->postError(
                   WP_VIEWPORT_ERROR_BAD_VALUE,
                   "Invalid source rect ({}, {}, {}, {}).",
                   srcRect.x(), srcRect.y(), srcRect.w(), srcRect.h());
                return false;
            }

            if (roundf((srcRect.x() + srcRect.w()) * Float32(current.bufferScale)) > sizeB.w() || roundf((srcRect.y() + srcRect.h()) * Float32(current.bufferScale)) > sizeB.h())
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
            srcRect.setX(0.f);
            srcRect.setY(0.f);
            srcRect.setW(Float32(sizeB.w()) / Float32(current.bufferScale));
            srcRect.setH(Float32(sizeB.h()) / Float32(current.bufferScale));
            stateFlags.remove(ViewportIsCropped);
        }

        // Using the viewport destination size
        if (surfaceResource->viewportRes()->dstSize().w() != -1 || surfaceResource->viewportRes()->dstSize().h() != -1)
        {
            size = surfaceResource->viewportRes()->dstSize();

            if (size.w() <= 0 || size.h() <= 0)
            {
                surfaceResource->viewportRes()->postError(
                    WP_VIEWPORT_ERROR_BAD_VALUE,
                    "Invalid destination size ({}, {}).",
                    size.w(), size.h());
                return false;
            }

            stateFlags.add(ViewportIsScaled);
        }

        // Using the viewport source rect size or normal surface size
        else
        {
            if (usingViewportSrc)
            {
                if (fmod(srcRect.w(), 1.f) != 0.f || fmod(srcRect.h(), 1.f) != 0.f)
                {
                    surfaceResource->viewportRes()->postError(
                        WP_VIEWPORT_ERROR_BAD_SIZE,
                        "Destination size is not integer");
                    return false;
                }

                size.setW(srcRect.w());
                size.setH(srcRect.h());
                stateFlags.add(ViewportIsScaled);
            }
            else
            {
                size.setW(roundf(srcRect.w()));
                size.setH(roundf(srcRect.h()));
                stateFlags.remove(ViewportIsScaled);
            }
        }
    }

    // Normal case, surface has no viewport
    else
    {
        srcRect.setX(0.f);
        srcRect.setY(0.f);
        srcRect.setW(Float32(sizeB.w()) / Float32(current.bufferScale));
        srcRect.setH(Float32(sizeB.h()) / Float32(current.bufferScale));
        size.setW(roundf(srcRect.w()));
        size.setH(roundf(srcRect.h()));
        stateFlags.remove(ViewportIsCropped);
        stateFlags.remove(ViewportIsScaled);
    }

    if (prevSize != size)
        changesToNotify.add(SizeChanged);

    if (prevSrcRect != srcRect)
        changesToNotify.add(SourceRectChanged);

    return true;
}

void LSurface::LSurfacePrivate::simplifyDamage(std::vector<LRect> &vec) noexcept
{
    if (vec.size() >= LOUVRE_MAX_DAMAGE_RECTS)
    {
        LBox extents;
        Int32 x2, y2;
        extents.x1 = vec.back().x();
        extents.y1 = vec.back().y();
        extents.x2 = extents.x1 + vec.back().w();
        extents.y2 = extents.y1 + vec.back().h();
        vec.pop_back();

        while (!vec.empty())
        {
            if (vec.back().x() < extents.x1)
                extents.x1 = vec.back().x();

            if (vec.back().y() < extents.y1)
                extents.y1 = vec.back().y();

            x2 = vec.back().x() + vec.back().w();

            if (x2 > extents.x2)
                extents.x2 = x2;

            y2 = vec.back().y() + vec.back().h();

            if (y2 > extents.y2)
                extents.y2 = y2;

            vec.pop_back();
        }

        vec.emplace_back(extents.x1, extents.y1, extents.x2 - extents.x1, extents.y2 - extents.y1);
    }
}

void LSurface::LSurfacePrivate::setLayer(LSurfaceLayer newLayer)
{
    for (LSurface *child : pendingChildren)
        if (!child->imp()->stateFlags.check(AboveParent) && (child->subsurface() || child->toplevel()))
            child->imp()->setLayer(newLayer);

    for (LSurface *child : children)
        if (!child->imp()->stateFlags.check(AboveParent) && (child->subsurface() || child->toplevel()))
            child->imp()->setLayer(newLayer);

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
        if (child->imp()->stateFlags.check(AboveParent) && (child->subsurface() || child->toplevel()))
            child->imp()->setLayer(layer);

    for (LSurface *child : children)
        if (child->imp()->stateFlags.check(AboveParent) && (child->subsurface() || child->toplevel()))
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
    assert(stateFlags.check(UnnotifiedRoleChange) && "There is no role change to notify");
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
        auto *role { surface->dndIcon() };
        compositor()->onAnticipatedObjectDestruction(role);
        surface->imp()->setMapped(false);
        surface->imp()->setRole(nullptr);
        surface->imp()->notifyRoleChange();
        delete role;
    }
    else if (surface->cursorRole())
    {
        auto *role { surface->cursorRole() };
        compositor()->onAnticipatedObjectDestruction(role);
        surface->imp()->setMapped(false);
        surface->imp()->setRole(nullptr);
        surface->imp()->notifyRoleChange();
        delete role;
    }
}

bool LSurface::LSurfacePrivate::canHostRole() const noexcept
{
    return role == nullptr && xdgSurface == nullptr;
}
