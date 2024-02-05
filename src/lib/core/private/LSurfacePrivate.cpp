#include <protocols/WpPresentationTime/private/RWpPresentationFeedbackPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>
#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <protocols/Wayland/private/GOutputPrivate.h>
#include <protocols/FractionalScale/RFractionalScale.h>
#include <private/LCompositorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LOutputMode.h>
#include <LClient.h>
#include <LTime.h>
#include <LLog.h>

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

    LSurface *surface = surfaceResource->surface();

    if (parent == nullptr)
    {
        this->parent->imp()->removeChild(surface);
        return;
    }

    this->parent = parent;

    if (parent->children().empty())
        compositor()->imp()->insertSurfaceAfter(parent, surface);
    else
        compositor()->imp()->insertSurfaceAfter(parent->children().back(), surface);

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

    LSurface *surface = surfaceResource->surface();

    if (stateFlags.check(Mapped) != state)
    {
        stateFlags.setFlag(Mapped, state);
        surface->mappingChanged();

        /* We create a copy of the childrens list
         * because a child could be removed
         * when handleParentMappingChange() is called */
        list<LSurface*> childrenTmp = children;

        for (LSurface *c : childrenTmp)
        {
            if (c->role())
                c->role()->handleParentMappingChange();
            else if (c->imp()->pending.role)
                c->imp()->pending.role->handleParentMappingChange();
        }
    }
}

void LSurface::LSurfacePrivate::setPendingRole(LBaseSurfaceRole *role)
{
    pending.role = role;
}

void LSurface::LSurfacePrivate::applyPendingRole()
{
    LSurface *surface = surfaceResource->surface();
    current.role = pending.role;
    pending.role = nullptr;
    surface->roleChanged();
}

void LSurface::LSurfacePrivate::applyPendingChildren()
{
    LSurface *surface = surfaceResource->surface();

    while (!pendingChildren.empty())
    {
        LSurface *child = pendingChildren.front();
        pendingChildren.pop_front();

        if (child->imp()->pendingParent != surface)
            continue;

        // If the child already had a parent
        if (child->imp()->parent)
            child->imp()->parent->imp()->children.erase(child->imp()->parentLink);

        if (surface->children().empty())
            compositor()->imp()->insertSurfaceAfter(surface, child);
        else
        {
            compositor()->imp()->insertSurfaceAfter(surface->children().back(), child);
        }

        children.push_back(child);
        child->imp()->pendingParent = nullptr;

        child->imp()->parent = surface;
        child->imp()->parentLink = std::prev(children.end());
        child->parentChanged();

        if (child->role())
            child->role()->handleParentChange();
        else if (child->imp()->pending.role)
            child->imp()->pending.role->handleParentChange();
    }
}

bool LSurface::LSurfacePrivate::bufferToTexture()
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

    // SHM
    if (wl_shm_buffer_get(current.buffer))
    {
        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = textureBackup;

        wl_shm_buffer *shm_buffer = wl_shm_buffer_get(current.buffer);
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
            texture->setDataB(LSize(widthB, heightB), stride, format, pixels);
        }
        else if (!pendingDamageB.empty() || !pendingDamage.empty())
        {
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

                UInt32 pixelSize = LTexture::formatBytesPerPixel(format);

                Int32 n;
                LBox *boxes = onlyPending.boxes(&n);
                LRect rect;
                for (Int32 i = 0; i < n; i++)
                {
                    rect.setX(boxes->x1);
                    rect.setY(boxes->y1);
                    rect.setW(boxes->x2 - boxes->x1);
                    rect.setH(boxes->y2 - boxes->y1);
                    texture->updateRect(rect,
                                        stride,
                                        &pixels[rect.x()*pixelSize + rect.y()*stride]);

                    boxes++;
                }

                onlyPending.transform(sizeB, LFramebuffer::requiredTransform(current.transform, LFramebuffer::Normal));
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

                UInt32 pixelSize = LTexture::formatBytesPerPixel(format);

                Int32 n;
                LBox *boxes = onlyPending.boxes(&n);
                LRect rect;
                for (Int32 i = 0; i < n; i++)
                {
                    rect.setX(boxes->x1);
                    rect.setY(boxes->y1);
                    rect.setW(boxes->x2 - boxes->x1);
                    rect.setH(boxes->y2 - boxes->y1);
                    texture->updateRect(rect,
                                        stride,
                                        &pixels[rect.x()*pixelSize + rect.y()*stride]);

                    boxes++;
                }

                LRegion::multiply(&currentDamage, &currentDamageB, 1.f/Float32(current.bufferScale));
            }
        }
        else
        {
            wl_shm_buffer_end_access(shm_buffer);
            return true;
        }

        wl_shm_buffer_end_access(shm_buffer);
    }

    // WL_DRM
    else if (compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.buffer, EGL_TEXTURE_FORMAT, &format))
    {
        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = textureBackup;
        compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.buffer, EGL_WIDTH, &widthB);
        compositor()->imp()->eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.buffer, EGL_HEIGHT, &heightB);
        updateDimensions(widthB, heightB);
        updateDamage();
        texture->setData(current.buffer);
    }

    // DMA-Buf
    else if (isDMABuffer(current.buffer))
    {
        LDMABuffer *dmaBuffer = (LDMABuffer*)wl_resource_get_user_data(current.buffer);
        widthB = dmaBuffer->planes()->width;
        heightB = dmaBuffer->planes()->height;

        updateDimensions(widthB, heightB);

        if (!dmaBuffer->texture())
        {
            dmaBuffer->imp()->texture = new LTexture();
            dmaBuffer->texture()->setDataB(dmaBuffer->planes());
        }

        updateDamage();

        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = dmaBuffer->texture();
    }
    else
    {
        LLog::error("[LSurfacePrivate::bufferToTexture] Unknown buffer type. Killing client.");
        wl_client_destroy(surfaceResource->client()->client());
        return false;
    }

    pendingDamageB.clear();
    pendingDamage.clear();
    wl_buffer_send_release(current.buffer);
    damageId = LTime::nextSerial();
    stateFlags.add(Damaged | BufferReleased);
    return true;
}

void LSurface::LSurfacePrivate::sendPresentationFeedback(LOutput *output)
{
    if (wpPresentationFeedbackResources.empty())
        return;

    // Check if the surface is visible in the given output
    for (LOutput *lOutput : surfaceResource->surface()->outputs())
    {
        if (lOutput != output)
            continue;

        for (Wayland::GOutput *gOutput : surfaceResource->client()->outputGlobals())
        {
            if (gOutput->output() != output)
                continue;

            while (!wpPresentationFeedbackResources.empty())
            {
                WpPresentationTime::RWpPresentationFeedback *rFeed = wpPresentationFeedbackResources.back();
                rFeed->sync_output(gOutput);
                rFeed->presented(output->imp()->presentationTime.time.tv_sec >> 32,
                                 output->imp()->presentationTime.time.tv_sec & 0xffffffff,
                                 (UInt32)output->imp()->presentationTime.time.tv_nsec,
                                 output->imp()->presentationTime.period,
                                 output->imp()->presentationTime.frame >> 32,
                                 output->imp()->presentationTime.frame & 0xffffffff,
                                output->imp()->presentationTime.flags);
                rFeed->imp()->lSurface = nullptr;
                wpPresentationFeedbackResources.pop_back();
                wl_resource_destroy(rFeed->resource());
            }

            return;
        }
    }

    while (!wpPresentationFeedbackResources.empty())
    {
        WpPresentationTime::RWpPresentationFeedback *rFeed = wpPresentationFeedbackResources.back();
        rFeed->discarded();
        rFeed->imp()->lSurface = nullptr;
        wpPresentationFeedbackResources.pop_back();
        wl_resource_destroy(rFeed->resource());
    }
}

void LSurface::LSurfacePrivate::sendPreferredScale()
{
    Int32 wlScale = 0;
    Float32 wlFracScale = 0.f;

    LFramebuffer::Transform transform = LFramebuffer::Normal;

    if (outputs.empty())
        return;

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

    if (surfaceResource->fractionalScaleResource())
        surfaceResource->fractionalScaleResource()->preferredScale(wlFracScale);
}

void LSurface::LSurfacePrivate::setPendingParent(LSurface *pendParent)
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

bool LSurface::LSurfacePrivate::isInChildrenOrPendingChildren(LSurface *child)
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

bool LSurface::LSurfacePrivate::hasRoleOrPendingRole()
{
    return current.role != nullptr || pending.role != nullptr;
}

bool LSurface::LSurfacePrivate::hasBufferOrPendingBuffer()
{
    return current.buffer != nullptr || pending.buffer != nullptr;
}

void LSurface::LSurfacePrivate::setKeyboardGrabToParent()
{
    if (seat()->keyboard()->grabbingSurface() && surfaceResource->surface() == seat()->keyboard()->grabbingSurface() && surfaceResource->surface()->parent())
    {
        if (surfaceResource->surface()->parent()->popup())
            seat()->keyboard()->setGrabbingSurface(surfaceResource->surface()->parent(), seat()->keyboard()->grabbingKeyboardResource());
        else
        {
            seat()->keyboard()->setGrabbingSurface(nullptr, nullptr);
            seat()->keyboard()->setFocus(surfaceResource->surface()->parent());
        }
    }
}
