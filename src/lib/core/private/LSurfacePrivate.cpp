#include <protocols/WpPresentationTime/private/RWpPresentationFeedbackPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>
#include <protocols/LinuxDMABuf/private/LDMABufferPrivate.h>
#include <protocols/Wayland/private/RSurfacePrivate.h>
#include <protocols/Wayland/private/GOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LTexturePrivate.h>
#include <private/LOutputPrivate.h>
#include <private/LKeyboardPrivate.h>
#include <LOutputMode.h>
#include <LLog.h>

static PFNEGLQUERYWAYLANDBUFFERWL eglQueryWaylandBufferWL = NULL;

void LSurface::LSurfacePrivate::getEGLFunctions()
{
    eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWL) eglGetProcAddress ("eglQueryWaylandBufferWL");
}

void LSurface::LSurfacePrivate::setParent(LSurface *parent)
{
    if (destroyed)
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
    if (destroyed)
        return;

    children.erase(child->imp()->parentLink);
    child->imp()->parent = nullptr;
    child->parentChanged();
}

void LSurface::LSurfacePrivate::setMapped(bool state)
{
    if (destroyed)
        return;

    LSurface *surface = surfaceResource->surface();

    if (mapped != state)
    {
        mapped = state;

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
    GLint texture_format;
    Int32 width, height;
    bool bufferScaleChanged = false;
    LSurface *surface = surfaceResource->surface();
    LSize prevSize = texture->sizeB();

    /***********************************
     *********** BUFFER SCALE ***********
     ***********************************/

    if (surface->imp()->current.bufferScale != surface->imp()->pending.bufferScale)
    {
        surface->imp()->current.bufferScale = surface->imp()->pending.bufferScale;
        bufferScaleChanged = true;
        surface->bufferScaleChanged();
    }

    // SHM
    if (wl_shm_buffer_get(current.buffer))
    {
        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = textureBackup;
        wl_shm_buffer *shm_buffer = wl_shm_buffer_get(current.buffer);
        wl_shm_buffer_begin_access(shm_buffer);
        width = wl_shm_buffer_get_width(shm_buffer);
        height = wl_shm_buffer_get_height(shm_buffer);
        void *data = wl_shm_buffer_get_data(shm_buffer);
        UInt32 format =  LTexture::waylandFormatToDRM(wl_shm_buffer_get_format(shm_buffer));
        Int32 stride = wl_shm_buffer_get_stride(shm_buffer);
        LSize newSize = LSize(width, height);

        if (texture->sizeB() != newSize)
            bufferSizeChanged = true;

        // Reemplaza toda la textura
        if (!texture->initialized() || bufferSizeChanged || bufferScaleChanged)
        {
            currentDamageB.clear();
            currentDamageB.addRect(LRect(0, newSize));
            currentDamage.clear();
            currentDamage.addRect(LRect(0, newSize / current.bufferScale));
            texture->setDataB(newSize, stride, format, data);
        }

        // Aplica daños
        else if (!pendingDamageB.empty() || !pendingDamage.empty())
        {
            LRegion onlyPending;

            if (current.bufferScale == 1)
            {
                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    onlyPending.addRect(r.x() - 1,
                                           r.y() - 1 ,
                                           r.w() + 2,
                                           r.h() + 2);
                    pendingDamage.pop_back();
                }
            }
            else if (current.bufferScale == 2)
            {
                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    onlyPending.addRect((r.x() - 1 ) << 1,
                                           (r.y() - 1 ) << 1,
                                           (r.w() + 2 ) << 1,
                                           (r.h() + 2 ) << 1),
                    pendingDamage.pop_back();
                }
            }
            else
            {
                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    onlyPending.addRect((r.x() - 1 )*current.bufferScale,
                                        (r.y() - 1 )*current.bufferScale,
                                        (r.w() + 2 )*current.bufferScale,
                                        (r.h() + 2 )*current.bufferScale);
                    pendingDamage.pop_back();
                }
            }

            if (current.bufferScale > 1)
            {
                Int32 modX, modY, modW, modH;
                while (!pendingDamageB.empty())
                {
                    LRect &r = pendingDamageB.back();
                    modX = r.x() % current.bufferScale;
                    modY = r.y() % current.bufferScale;
                    modW = r.w() % current.bufferScale;
                    modH = r.h() % current.bufferScale;
                    onlyPending.addRect(
                                r.x() - modX,
                                r.y() - modY,
                                r.w() + modW + (modX << 1),
                                r.h() + modH + (modY << 1));
                    pendingDamageB.pop_back();
                }
            }
            else
            {
                while (!pendingDamageB.empty())
                {

                    onlyPending.addRect(pendingDamageB.back());
                    pendingDamageB.pop_back();
                }
            }

            onlyPending.clip(LRect(0, texture->sizeB()));

            UInt32 pixelSize = LTexture::formatBytesPerPixel(format);
            UChar8 *buff = (UChar8 *)data;

            Int32 n, w, h;
            LBox *boxes = onlyPending.rects(&n);
            for (Int32 i = 0; i < n; i++)
            {
                w = boxes->x2 - boxes->x1;
                h = boxes->y2 - boxes->y1;
                texture->updateRect(LRect(boxes->x1,
                                          boxes->y1,
                                          w,
                                          h),
                                    stride,
                                    &buff[boxes->x1*pixelSize + boxes->y1*stride]);

                currentDamageB.addRect(boxes->x1, boxes->y1, w, h);
                boxes++;
            }

            currentDamage = currentDamageB;
            currentDamage.multiply(1.f/float(current.bufferScale));
        }
        else
        {
            wl_shm_buffer_end_access(shm_buffer);
            return true;
        }

        wl_shm_buffer_end_access(shm_buffer);
    }
    // WL_DRM
    else if (eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.buffer, EGL_TEXTURE_FORMAT, &texture_format))
    {
        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = textureBackup;

        eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.buffer, EGL_WIDTH, &width);
        eglQueryWaylandBufferWL(LCompositor::eglDisplay(), current.buffer, EGL_HEIGHT, &height);

        LSize newSize = LSize(width, height);

        if (newSize != prevSize || bufferScaleChanged)
        {
            bufferSizeChanged = true;
            currentDamageB.clear();
            currentDamageB.addRect(LRect(0, newSize));
            currentDamage.clear();
            currentDamage.addRect(LRect(0, newSize / current.bufferScale));
        }
        else if (!pendingDamageB.empty() || !pendingDamage.empty())
        {
            if (current.bufferScale == 1)
            {
                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    currentDamageB.addRect(r.x() - 1,
                                           r.y() - 1 ,
                                           r.w() + 2,
                                           r.h() + 2);
                    pendingDamage.pop_back();
                }
            }
            else if (current.bufferScale == 2)
            {
                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    currentDamageB.addRect((r.x() - 1 ) << 1,
                                           (r.y() - 1 ) << 1,
                                           (r.w() + 2 ) << 1,
                                           (r.h() + 2 ) << 1),
                    pendingDamage.pop_back();
                }
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
            }

            if (current.bufferScale > 1)
            {
                Int32 modX, modY, modW, modH;
                while (!pendingDamageB.empty())
                {
                    LRect &r = pendingDamageB.back();
                    modX = r.x() % current.bufferScale;
                    modY = r.y() % current.bufferScale;
                    modW = r.w() % current.bufferScale;
                    modH = r.h() % current.bufferScale;
                    currentDamageB.addRect(
                                r.x() - modX,
                                r.y() - modY,
                                r.w() + modW + (modX << 1),
                                r.h() + modH + (modY << 1));
                    pendingDamageB.pop_back();
                }
            }
            else
            {
                while (!pendingDamageB.empty())
                {

                    currentDamageB.addRect(pendingDamageB.back());
                    pendingDamageB.pop_back();
                }
            }

            currentDamageB.clip(LRect(0, newSize));
            currentDamage = currentDamageB;
            currentDamage.multiply(1.f/float(current.bufferScale));
        }

        texture->setData(current.buffer);
    }
    else if (isDMABuffer(current.buffer))
    {
        LDMABuffer *dmaBuffer = (LDMABuffer*)wl_resource_get_user_data(current.buffer);
        width = dmaBuffer->planes()->width;
        height = dmaBuffer->planes()->height;

        if (!dmaBuffer->texture())
        {
            dmaBuffer->imp()->texture = new LTexture();
            dmaBuffer->texture()->setDataB(dmaBuffer->planes());
        }

        LSize newSize = LSize(width, height);

        if (newSize != prevSize || bufferScaleChanged || !texture->initialized())
        {
            bufferSizeChanged = true;
            currentDamageB.clear();
            currentDamageB.addRect(LRect(0, newSize));
            currentDamage.clear();
            currentDamage.addRect(LRect(0, newSize / current.bufferScale));
        }
        else if (!pendingDamageB.empty() || !pendingDamage.empty())
        {
            if (current.bufferScale == 1)
            {
                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    currentDamageB.addRect(r.x() - 1,
                                           r.y() - 1 ,
                                           r.w() + 2,
                                           r.h() + 2);
                    pendingDamage.pop_back();
                }
            }
            else if (current.bufferScale == 2)
            {
                while (!pendingDamage.empty())
                {
                    LRect &r = pendingDamage.back();
                    currentDamageB.addRect((r.x() - 1 ) << 1,
                                           (r.y() - 1 ) << 1,
                                           (r.w() + 2 ) << 1,
                                           (r.h() + 2 ) << 1),
                    pendingDamage.pop_back();
                }
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
            }


            if (current.bufferScale > 1)
            {
                Int32 modX, modY, modW, modH;
                while (!pendingDamageB.empty())
                {
                    LRect &r = pendingDamageB.back();
                    modX = r.x() % current.bufferScale;
                    modY = r.y() % current.bufferScale;
                    modW = r.w() % current.bufferScale;
                    modH = r.h() % current.bufferScale;
                    currentDamageB.addRect(
                                r.x() - modX,
                                r.y() - modY,
                                r.w() + modW +  (modX << 1),
                                r.h() + modH +  (modY << 1));
                    pendingDamageB.pop_back();
                }
            }
            else
            {
                while (!pendingDamageB.empty())
                {
                    currentDamageB.addRect(pendingDamageB.back());
                    pendingDamageB.pop_back();
                }
            }

            currentDamageB.clip(LRect(0, newSize));
            currentDamage = currentDamageB;
            currentDamage.multiply(1.f/float(current.bufferScale));
        }

        if (texture && texture != textureBackup && texture->imp()->pendingDelete)
            delete texture;

        texture = dmaBuffer->texture();
    }
    else
    {
        LLog::error("[surface] Unknown buffer type. Killing client.");
        wl_client_destroy(surface->client()->client());
        return false;
    }

    currentSizeB = texture->sizeB();
    currentSize = texture->sizeB()/current.bufferScale;

    if (bufferSizeChanged)
        surface->bufferSizeChanged();

    pendingDamageB.clear();
    pendingDamage.clear();

    wl_buffer_send_release(current.buffer);
    bufferReleased = true;

    damageId = LCompositor::nextSerial();

    damaged = true;
    surface->damageChanged();
    return true;
}

void LSurface::LSurfacePrivate::sendPresentationFeedback(LOutput *output, timespec &ns)
{
    if (wpPresentationFeedbackResources.empty())
        return;

    // Check if the surface is visible in the given output
    for (LOutput *lOutput : surfaceResource->surface()->outputs())
    {
        if (lOutput == output)
        {
            for (Wayland::GOutput *gOutput : surfaceResource->client()->outputGlobals())
            {
                if (gOutput->output() == output)
                {
                    UInt32 tv_sec_hi = ns.tv_sec >> 32;
                    UInt32 tv_sec_lo = ns.tv_sec & 0xffffffff;
                    UInt32 seq_hi = 0; /* >> 32; */
                    UInt32 seq_lo = 0; /* & 0xFFFFFFFF; */
                    UInt32 refresh = 0; // 1000000000/output->currentMode()->refreshRate();

                    while (!wpPresentationFeedbackResources.empty())
                    {
                        WpPresentationTime::RWpPresentationFeedback *rFeed = wpPresentationFeedbackResources.back();
                        rFeed->sync_output(gOutput);
                        rFeed->presented(tv_sec_hi,
                                         tv_sec_lo,
                                         ns.tv_nsec,
                                         refresh,
                                         seq_hi,
                                         seq_lo,
                                         WP_PRESENTATION_FEEDBACK_KIND_VSYNC);
                        rFeed->imp()->lSurface = nullptr;
                        wpPresentationFeedbackResources.pop_back();
                        wl_resource_destroy(rFeed->resource());
                    }

                    return;
                }
            }
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
    Int32 scale = 1;

    for (LOutput *o : outputs)
    {
        if (o->scale() > scale)
            scale = o->scale();
    }

    if (lastSentPreferredBufferScale == scale)
        return;

    lastSentPreferredBufferScale = scale;
    surfaceResource->preferredBufferScale(scale);
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
