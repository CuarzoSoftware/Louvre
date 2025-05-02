#include <protocols/Wayland/GOutput.h>
#include <protocols/ScreenCopy/RScreenCopyFrame.h>
#include <protocols/ScreenCopy/GScreenCopyManager.h>
#include <protocols/SessionLock/RSessionLock.h>
#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LClientPrivate.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <LExclusiveZone.h>
#include <LOutputMode.h>
#include <LLayerRole.h>
#include <LSeat.h>
#include <LGlobal.h>
#include <LTime.h>

using namespace Louvre::Protocols::Wayland;

LOutput::LOutputPrivate::LOutputPrivate(LOutput *output) : fb(output) {}

// This is called from LCompositor::addOutput()
bool LOutput::LOutputPrivate::initialize()
{
    output->imp()->state = LOutput::PendingInitialize;
    return compositor()->imp()->graphicBackend->outputInitialize(output);
}

void LOutput::LOutputPrivate::backendInitializeGL()
{
    threadId = std::this_thread::get_id();
    painter.reset(&compositor()->imp()->initThreadData(output).painter);
    compositor()->imp()->disablePendingPosixSignals();
    output->imp()->global.reset(compositor()->createGlobal<Protocols::Wayland::GOutput>(output));
    output->setScale(output->imp()->fractionalScale);
    lastPos = rect.pos();
    lastSize = rect.size();
    cursor()->imp()->textureChanged = true;
    cursor()->imp()->update();
    output->imp()->state = LOutput::Initialized;

    if (sessionLockRole && sessionLockRole->surface())
        sessionLockRole->surface()->imp()->setMapped(true);

    updateLayerSurfacesMapping();

    output->initializeGL();
    compositor()->flushClients();
    initACK = true;
}

void LOutput::LOutputPrivate::damageToBufferCoords() noexcept
{
    cursor()->enableHwCompositing(output, screenshotCursorTimeout == 0);

    if (screenshotCursorTimeout > 0 && cursor()->visible() && cursor()->enabled(output))
    {
        damage.addRect(LRect(prevCursorRect.pos() + output->pos(), prevCursorRect.size()));
        stateFlags.add(CursorNeedsRendering);
    }

    if (!damage.empty() &&
        (stateFlags.checkAll(UsingFractionalScale | FractionalOversamplingEnabled) ||
         output->hasBufferDamageSupport() ||
         compositor()->imp()->screenshotManagers > 0 ||
         stateFlags.check(ScreenshotsWithCursor | ScreenshotsWithoutCursor)))
    {
        damage.offset(-rect.pos().x(), -rect.pos().y());
        damage.transform(rect.size(), transform);

        LRegion tmp; Int32 n;
        const LBox *box { damage.boxes(&n) };
        while (n > 0)
        {
            tmp.addRect(floorf(Float32(box->x1) * fractionalScale) - 2, floorf(Float32(box->y1) * fractionalScale) - 2,
                        ceilf(Float32(box->x2 - box->x1) * fractionalScale) + 4, ceilf(Float32(box->y2 - box->y1) * fractionalScale) + 4);
            n--; box++;
        }

        damage = std::move(tmp);
        damage.clip(LRect(0, output->currentMode()->sizeB()));

        if (output->hasBufferDamageSupport())
            compositor()->imp()->graphicBackend->outputSetBufferDamage(output, damage);

        if (compositor()->imp()->screenshotManagers > 0)
        {
            for (LClient *client : compositor()->clients())
            {
                for (auto *screenCpyManager : client->imp()->screenCopyManagerGlobals)
                {
                    auto &outputDamage { screenCpyManager->damage[output] };

                    if (!outputDamage.firstFrame)
                        outputDamage.damage.addRegion(damage);
                }
            }
        }
    }
}

void LOutput::LOutputPrivate::blitFractionalScaleFb(bool cursorOnly) noexcept
{
    stateFlags.remove(UsingFractionalScale);
    const LTransform prevTrasform { transform };
    transform = LTransform::Normal;
    const Float32 prevScale { scale };
    const Float32 prevFracScale { fractionalScale };
    scale = 1.f;
    const LPoint prevPos { rect.pos() };
    const LSize prevSize { rect.size() };
    rect.setPos(LPoint(0));
    updateRect();
    painter->bindFramebuffer(&fb);
    painter->enableCustomTextureColor(false);
    painter->enableAutoBlendFunc(true);
    painter->setAlpha(1.f);
    painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
    fractionalFb.setFence();
    painter->bindTextureMode({
        .texture = fractionalFb.texture(0),
        .pos = rect.pos(),
        .srcRect = LRect(0, fractionalFb.sizeB()),
        .dstSize = rect.size(),
        .srcTransform = LTransform::Normal,
        .srcScale = 1.f
    });

    glDisable(GL_BLEND);

    if (cursorOnly)
    {
        LRegion cursorDamage { prevCursorRect };
        cursorDamage.transform(prevSize, prevTrasform);
        LRegion tmp; Int32 n;
        const LBox *box { cursorDamage.boxes(&n) };
        while (n > 0)
        {
            tmp.addRect(floorf(Float32(box->x1) * prevFracScale) - 2, floorf(Float32(box->y1) * prevFracScale) - 2,
                        ceilf(Float32(box->x2 - box->x1) * prevFracScale) + 4, ceilf(Float32(box->y2 - box->y1) * prevFracScale) + 4);
            n--; box++;
        }

        cursorDamage = std::move(tmp);
        cursorDamage.clip(LRect(0, output->currentMode()->sizeB()));

        painter->drawRegion(cursorDamage);
    }
    else
        painter->drawRegion(damage);

    stateFlags.add(UsingFractionalScale);
    transform = prevTrasform;
    scale = prevScale;
    rect.setPos(prevPos);
    updateRect();
}

void LOutput::LOutputPrivate::handleScreenshotRequests(bool withCursor) noexcept
{
    for (std::size_t i = 0; i < screenshotRequests.size();)
    {
        if (screenshotRequests[i]->resource().compositeCursor() == withCursor)
        {
            // Wait for damage
            if (screenshotRequests[i]->copy() == 0)
                screenshotRequests[i]->resource().m_stateFlags.remove(ScreenCopy::RScreenCopyFrame::Accepted);

            // Fail or success
            else
            {
                screenshotRequests[i] = screenshotRequests.back();
                screenshotRequests.pop_back();
                continue;
            }
        }

        i++;
    }
}

void LOutput::LOutputPrivate::blitFramebuffers() noexcept
{
    if (stateFlags.checkAll(UsingFractionalScale | FractionalOversamplingEnabled))
    {
        if (stateFlags.checkAll(ScreenshotsWithCursor | ScreenshotsWithoutCursor))
        {
            blitFractionalScaleFb(false);
            handleScreenshotRequests(false);
            painter->bindFramebuffer(&fractionalFb);
            drawCursor();
            blitFractionalScaleFb(true);
            handleScreenshotRequests(true);
        }
        else if (stateFlags.check(ScreenshotsWithCursor) && !stateFlags.check(ScreenshotsWithoutCursor))
        {
            drawCursor();
            blitFractionalScaleFb(false);
            handleScreenshotRequests(true);
        }
        else if (!stateFlags.check(ScreenshotsWithCursor) && stateFlags.check(ScreenshotsWithoutCursor))
        {
            blitFractionalScaleFb(false);
            handleScreenshotRequests(false);
            drawCursor();
            blitFractionalScaleFb(true);
        }
        else
        {
            drawCursor();
            blitFractionalScaleFb(false);
        }
    }
    else
    {
        if (stateFlags.checkAll(ScreenshotsWithCursor | ScreenshotsWithoutCursor))
        {
            handleScreenshotRequests(false);
            painter->bindFramebuffer(&fb);
            drawCursor();
            handleScreenshotRequests(true);
        }
        else if (stateFlags.check(ScreenshotsWithCursor) && !stateFlags.check(ScreenshotsWithoutCursor))
        {
            drawCursor();
            handleScreenshotRequests(true);
        }
        else if (!stateFlags.check(ScreenshotsWithCursor) && stateFlags.check(ScreenshotsWithoutCursor))
        {
            handleScreenshotRequests(false);
            drawCursor();
        }
        else
        {
            drawCursor();
        }
    }
}

void LOutput::LOutputPrivate::backendPaintGL()
{
    bool callLock = output->imp()->callLock.load();

    if (!callLock)
        callLockACK.store(true);

    if (output->imp()->state != LOutput::Initialized)
        return;

    if (callLock)
        compositor()->imp()->lock();

    compositor()->imp()->disablePendingPosixSignals();

    if (!repaintFilter())
    {
        compositor()->imp()->graphicBackend->outputLockCurrentBuffer(output, true);

        if (callLock)
            compositor()->imp()->unlock();

        return;
    }

    stateFlags.remove(PendingRepaint);

    if (seat()->enabled() && compositor()->imp()->runningAnimations())
    {
        output->repaint();
        compositor()->imp()->unlockPoll();
    }

    if (lastPos != rect.pos())
    {
        output->moveGL();
        lastPos = rect.pos();
    }

    if (lastSize != rect.size())
    {
        output->resizeGL();
        lastSize = rect.size();
    }

    // Send presentation time of the prev frame
    compositor()->imp()->sendPresentationTime();

    // Update active LAnimations
    compositor()->imp()->processAnimations();

    painter->bindFramebuffer(&fb);
    compositor()->imp()->currentOutput = output;

    /* Mark the entire output rect as damaged for compositors
     * that do not track damage.*/
    damage.clear();
    damage.addRect(output->rect());

    /* Add the region the user should repaint if hw cursor
     * is disabled */
    calculateCursorDamage();

    const bool needsFullRepaintPrev { stateFlags.check(NeedsFullRepaint) };

    /* If the last frame had a custom scanout buffer and the current
     * one doesn't, suggest a full repaint */
    if (stateFlags.check(HasScanoutBuffer))
    {
        stateFlags.add(NeedsFullRepaint);
        stateFlags.remove(HasScanoutBuffer);
    }

    /* Release prev scanout buffer (1 before the one being scanned out rn) */
    releaseScanoutBuffer(1);

    /* Move current scanout buffer to 1 */
    if (scanout[0].buffer)
    {
        wl_list_remove(&scanout[0].bufferDestroyListener.link);
        scanout[1].buffer = scanout[0].buffer;
        scanout[1].surface = scanout[0].surface;
        wl_resource_add_destroy_listener((wl_resource*)scanout[1].buffer, &scanout[1].bufferDestroyListener);
        scanout[0].buffer = nullptr;
        scanout[0].surface.reset();
    }

    /* Let users do their rendering*/
    painter->bindProgram();
    painter->bindFramebuffer(&fb);
    stateFlags.add(IsInPaintGL);
    output->paintGL();
    stateFlags.remove(IsInPaintGL);
    compositor()->imp()->graphicBackend->outputLockCurrentBuffer(output, false);
    painter->bindProgram();
    painter->bindFramebuffer(&fb);

    /* Force repaint if there are unreleased buffers */
    if (scanout[0].buffer || scanout[1].buffer)
        output->repaint();

    stateFlags.setFlag(NeedsFullRepaint, needsFullRepaintPrev);

    /* This ensures that all active outputs have been repainted at least once after a client requests to lock the session.
     * Protocol quote: The locked event must not be sent until a new "locked" frame has been presented on all outputs and no
     * security sensitive normal/unlocked content is possibly visible. */
    removeFromSessionLockPendingRepaint();

    /* Remove denied or invalid screen copy requests */
    validateScreenshotRequests();

    compositor()->imp()->currentOutput = nullptr;

    if (!stateFlags.check(HasScanoutBuffer))
    {
        /* Turn damage into buffer coords and handle buffer
         * blitting if oversampling is enabled or there are
         * screen copy requests*/
        stateFlags.add(IsBlittingFramebuffers);
        damageToBufferCoords();
        blitFramebuffers();
        stateFlags.remove(IsBlittingFramebuffers);
    }

    /* Ensure clients receive frame callbacks and pending roles configurations on time */
    compositor()->flushClients();

    /* Destroy render buffers created from this thread and marked as destroyed by the user */
    compositor()->imp()->destroyPendingRenderBuffers(&output->imp()->threadId);

    if (callLock)
        compositor()->imp()->unlock();
}

void LOutput::LOutputPrivate::backendResizeGL()
{
    bool callLock = output->imp()->callLock.load();

    if (!callLock)
        callLockACK.store(true);

    if (output->imp()->state == LOutput::ChangingMode)
    {
        output->imp()->state = LOutput::Initialized;
        output->setScale(output->fractionalScale());
        output->imp()->updateRect();
        output->imp()->updateGlobals();
        cursor()->imp()->textureChanged = true;
    }

    if (output->imp()->state != LOutput::Initialized)
        return;

    if (callLock)
        compositor()->imp()->lock();

    compositor()->imp()->disablePendingPosixSignals();

    output->resizeGL();

    if (lastPos != rect.pos())
    {
        output->moveGL();
        lastPos = rect.pos();
    }

    if (callLock)
        compositor()->imp()->unlock();
}

void LOutput::LOutputPrivate::backendUninitializeGL()
{
    bool callLock = output->imp()->callLock.load();

    if (!callLock)
        callLockACK.store(true);

    if (output->imp()->state != LOutput::PendingUninitialize)
       return;

    if (callLock)
       compositor()->imp()->lock();

    compositor()->imp()->disablePendingPosixSignals();

    if (sessionLockRole && sessionLockRole->surface())
       sessionLockRole->surface()->imp()->setMapped(false);

    while (!screenshotRequests.empty())
    {
       screenshotRequests.back()->resource().failed();
       screenshotRequests.pop_back();
    }

    output->uninitializeGL();
    removeFromSessionLockPendingRepaint();

    /* Just in case there is a pending user buffer release */
    releaseScanoutBuffer(0);
    releaseScanoutBuffer(1);

    compositor()->flushClients();
    output->imp()->state = LOutput::Uninitialized;
    updateLayerSurfacesMapping();
    compositor()->imp()->destroyPendingRenderBuffers(&output->imp()->threadId);
    compositor()->imp()->unitThreadData();

    if (callLock)
        compositor()->imp()->unlock();
}

void LOutput::LOutputPrivate::backendPageFlipped()
{
    pageflipMutex.lock();
    stateFlags.add(HasUnhandledPresentationTime);
    frame++;
    pageflipMutex.unlock();
}

void LOutput::LOutputPrivate::updateRect()
{
    if (stateFlags.check(UsingFractionalScale))
    {
        sizeB = compositor()->imp()->graphicBackend->outputGetCurrentMode(output)->sizeB();
        sizeB.setW(roundf(Float32(sizeB.w()) * scale / fractionalScale));
        sizeB.setH(roundf(Float32(sizeB.h()) * scale / fractionalScale));
    }
    else
        sizeB = compositor()->imp()->graphicBackend->outputGetCurrentMode(output)->sizeB();

    // Swap width with height
    if (Louvre::is90Transform(transform))
    {
        Int32 tmpW = sizeB.w();
        sizeB.setW(sizeB.h());
        sizeB.setH(tmpW);
    }

    rect.setSize(sizeB);
    rect.setW(roundf(Float32(rect.w())/scale));
    rect.setH(roundf(Float32(rect.h())/scale));

    if (!stateFlags.check(IsBlittingFramebuffers))
        updateExclusiveZones();
}

void LOutput::LOutputPrivate::updateGlobals()
{
    for (LClient *c : compositor()->clients())
        for (GOutput *global : c->outputGlobals())
            if (output == global->output())
                global->sendConfiguration();

    for (LSurface *s : compositor()->surfaces())
        s->imp()->sendPreferredScale();

    if (output->sessionLockRole())
        output->sessionLockRole()->configure(output->size());
}

void LOutput::LOutputPrivate::calculateCursorDamage() noexcept
{
    cursorDamage.clear();

    if (cursor()->enabled(output) && !cursor()->hwCompositingEnabled(output) && cursor()->visible())
    {
        stateFlags.add(CursorNeedsRendering);
        cursorDamage.addRect(cursor()->rect());
        cursorDamage.addRect(LRect(prevCursorRect.pos() + output->pos(), prevCursorRect.size()));
        cursorDamage.clip(output->rect());
        dirtyCursorFBs = output->buffersCount();
    }

    if (stateFlags.check(CursorRenderedInPrevFrame) && cursor()->hwCompositingEnabled(output))
    {
        cursorDamage.addRect(LRect(prevCursorRect.pos() + output->pos(), prevCursorRect.size()));
        output->repaint();

        if (dirtyCursorFBs > 0)
            dirtyCursorFBs--;
        else
            stateFlags.remove(CursorRenderedInPrevFrame);
    }

    prevCursorRect = LRect(cursor()->rect().pos() - output->pos(), cursor()->rect().size());
}

void LOutput::LOutputPrivate::drawCursor() noexcept
{
    // Manualy draw the cursor if hardware composition is not supported
    if (stateFlags.check(CursorNeedsRendering))
    {
        glEnable(GL_BLEND);
        stateFlags.remove(CursorNeedsRendering);
        stateFlags.add(CursorRenderedInPrevFrame);
        painter->enableCustomTextureColor(false);
        painter->enableAutoBlendFunc(true);
        painter->setAlpha(1.f);
        painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
        painter->bindTextureMode(
        {
            .texture = cursor()->texture(),
            .pos = cursor()->rect().pos(),
            .srcRect = LRect(0, cursor()->texture()->sizeB()),
            .dstSize = cursor()->rect().size(),
            .srcTransform = LTransform::Normal,
            .srcScale = 1.f
        });
        painter->drawRect(cursor()->rect());
    }
}

void LOutput::LOutputPrivate::validateScreenshotRequests() noexcept
{
    stateFlags.remove(ScreenshotsWithCursor | ScreenshotsWithoutCursor);
    for (std::size_t i = 0; i < screenshotRequests.size();)
    {
        if (stateFlags.check(HasScanoutBuffer) || // Deny screenshoots when there is a custom scanout buffer
            !screenshotRequests[i]->resource().accepted() ||
            !screenshotRequests[i]->resource().m_bufferContainer.buffer ||
            screenshotRequests[i]->resource().m_initOutputModeSize != output->currentMode()->sizeB() ||
            screenshotRequests[i]->resource().m_initOutputSize != output->size() ||
            screenshotRequests[i]->resource().m_initOutputTransform != output->transform())
        {
            screenshotRequests[i]->resource().failed();
            screenshotRequests[i] = screenshotRequests.back();
            screenshotRequests.pop_back();
        }
        else
        {
            if (screenshotRequests[i]->resource().compositeCursor())
                stateFlags.add(ScreenshotsWithCursor);
            else
                stateFlags.add(ScreenshotsWithoutCursor);

            i++;
        }
    }

    if (stateFlags.check(ScreenshotsWithCursor))
        screenshotCursorTimeout = 5;
    else if (screenshotCursorTimeout > 0)
        screenshotCursorTimeout--;
}

void LOutput::LOutputPrivate::removeFromSessionLockPendingRepaint() noexcept
{
    if (sessionLockManager()->state() == LSessionLockManager::Locked && !sessionLockManager()->m_sessionLockRes->m_lockedOnce)
    {
        LVectorRemoveOneUnordered(sessionLockManager()->m_sessionLockRes->m_pendingRepaint, output);

        if (sessionLockManager()->m_sessionLockRes->m_pendingRepaint.empty())
            sessionLockManager()->m_sessionLockRes->locked();
    }
}

void LOutput::LOutputPrivate::updateExclusiveZones() noexcept
{
    exclusiveEdges = {0, 0, 0, 0};

    LRect prev;

    for (LExclusiveZone *zone : exclusiveZones)
    {
        if (zone->size() <= 0)
            continue;

        if (zone->m_onRectChangeCallback)
            prev = zone->m_rect;

        switch (zone->edge())
        {
        case LEdgeNone:
            break;
        case LEdgeLeft:
            zone->m_rect.setX(exclusiveEdges.left);
            zone->m_rect.setY(exclusiveEdges.top);
            zone->m_rect.setW(zone->size());
            zone->m_rect.setH(rect.h() - exclusiveEdges.top - exclusiveEdges.bottom);
            exclusiveEdges.left += zone->size();
            break;
        case LEdgeTop:
            zone->m_rect.setX(exclusiveEdges.left);
            zone->m_rect.setY(exclusiveEdges.top);
            zone->m_rect.setH(zone->size());
            zone->m_rect.setW(rect.w() - exclusiveEdges.left - exclusiveEdges.right);
            exclusiveEdges.top += zone->size();
            break;
        case LEdgeRight:
            zone->m_rect.setY(exclusiveEdges.top);
            zone->m_rect.setH(rect.h() - exclusiveEdges.top - exclusiveEdges.bottom);
            zone->m_rect.setW(zone->size());
            exclusiveEdges.right += zone->size();
            zone->m_rect.setX(rect.w() - exclusiveEdges.right);
            break;
        case LEdgeBottom:
            zone->m_rect.setX(exclusiveEdges.left);
            zone->m_rect.setW(rect.w() - exclusiveEdges.left - exclusiveEdges.right);
            zone->m_rect.setH(zone->size());
            exclusiveEdges.bottom += zone->size();
            zone->m_rect.setY(rect.h() - exclusiveEdges.bottom);
            break;
        }

        if (zone->m_onRectChangeCallback && prev != zone->m_rect)
            zone->m_onRectChangeCallback(zone);
    }

    prev = availableGeometry;
    availableGeometry.setX(exclusiveEdges.left);
    availableGeometry.setY(exclusiveEdges.top);
    availableGeometry.setW(rect.w() - exclusiveEdges.left - exclusiveEdges.right);
    availableGeometry.setH(rect.h() - exclusiveEdges.top - exclusiveEdges.bottom);

    const bool availableGeometryChanged { availableGeometry != prev };

    for (LExclusiveZone *zone : exclusiveZones)
    {
        if (zone->m_onRectChangeCallback)
            prev = zone->m_rect;

        if (zone->edge() == LEdgeNone)
        {
            if (zone->size() >= 0)
                zone->m_rect = availableGeometry;
            else
                zone->m_rect = LRect(0, rect.size());
        }
        else
        {
            if (zone->size() == 0)
                zone->m_rect = availableGeometry;
            else if ( zone->size() < 0)
                zone->m_rect = LRect(0, rect.size());
        }

        if (zone->m_onRectChangeCallback && prev != zone->m_rect)
            zone->m_onRectChangeCallback(zone);
    }

    if (availableGeometryChanged)
        output->availableGeometryChanged();
}

void LOutput::LOutputPrivate::updateLayerSurfacesMapping() noexcept
{
    for (LSurface *s : compositor()->surfaces())
        if (s->layerRole() && s->layerRole()->exclusiveOutput() == output)
            s->layerRole()->updateMappingState();
}

bool LOutput::LOutputPrivate::isBufferScannedByOtherOutputs(wl_buffer *buffer) const noexcept
{
    if (!buffer)
        return false;

    for (LOutput *o : compositor()->outputs())
    {
        if (o == output)
            continue;

        if (o->imp()->scanout[0].buffer == buffer || o->imp()->scanout[1].buffer == buffer)
            return true;
    }

    return false;
}

void LOutput::LOutputPrivate::releaseScanoutBuffer(UInt8 index) noexcept
{
    if (scanout[index].buffer)
    {
        wl_list_remove(&scanout[index].bufferDestroyListener.link);

        /* Other output will release it */
        if (isBufferScannedByOtherOutputs(scanout[index].buffer))
            goto skipRelease;

        /* The surface will release it */
        if (scanout[index].surface && scanout[index].surface->bufferResource() == scanout[index].buffer)
            goto skipRelease;

        /* The output will release it later */
        if (index == 1 && scanout[0].buffer == scanout[index].buffer)
            goto skipRelease;

        wl_buffer_send_release((wl_resource*)scanout[index].buffer);
    }

skipRelease:
    scanout[index].buffer = nullptr;
    scanout[index].surface.reset();
}
