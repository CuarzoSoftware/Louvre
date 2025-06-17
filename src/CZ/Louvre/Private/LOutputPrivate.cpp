#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Protocols/ScreenCopy/RScreenCopyFrame.h>
#include <CZ/Louvre/Protocols/ScreenCopy/GScreenCopyManager.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLock.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LPainterPrivate.h>
#include <CZ/Louvre/Private/LCursorPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Utils/CZRegionUtils.h>
#include <CZ/Louvre/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/LExclusiveZone.h>
#include <CZ/Louvre/LOutputMode.h>
#include <CZ/Louvre/Roles/LLayerRole.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LGlobal.h>
#include <CZ/Louvre/LTime.h>

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
    output->imp()->global.reset(compositor()->createGlobal<Protocols::Wayland::GOutput>(0, output));
    output->setScale(output->imp()->fractionalScale);
    lastPos = rect.topLeft();
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
        damage.op(
            SkIRect::MakePtSize(prevCursorRect.topLeft() + output->pos(), prevCursorRect.size()),
            SkRegion::kUnion_Op);
        stateFlags.add(CursorNeedsRendering);
    }

    if (!damage.isEmpty() &&
        (stateFlags.hasAll(UsingFractionalScale | FractionalOversamplingEnabled) ||
         output->hasBufferDamageSupport() ||
         compositor()->imp()->screenshotManagers > 0 ||
         stateFlags.has(ScreenshotsWithCursor | ScreenshotsWithoutCursor)))
    {
        damage.translate(-rect.x(), -rect.y());
        CZRegionUtils::ApplyTransform(damage, rect.size(), transform);

        SkRegion tmp;
        SkRegion::Iterator it(damage);

        while (!it.done())
        {
            tmp.op(
                SkIRect::MakeXYWH(
                SkScalarFloorToInt(Float32(it.rect().x()) * fractionalScale) - 2,
                SkScalarFloorToInt(Float32(it.rect().y()) * fractionalScale) - 2,
                SkScalarCeilToInt(Float32(it.rect().width()) * fractionalScale) + 4,
                SkScalarCeilToInt(Float32(it.rect().height()) * fractionalScale) + 4),
                SkRegion::kUnion_Op);
            it.next();
        }

        damage.op(SkIRect::MakeSize(output->currentMode()->sizeB()), std::move(tmp), SkRegion::kIntersect_Op);

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
                        outputDamage.damage.op(damage, SkRegion::kUnion_Op);
                }
            }
        }
    }
}

void LOutput::LOutputPrivate::blitFractionalScaleFb(bool cursorOnly) noexcept
{
    stateFlags.remove(UsingFractionalScale);
    const CZTransform prevTrasform { transform };
    transform = CZTransform::Normal;
    const Float32 prevScale { scale };
    const Float32 prevFracScale { fractionalScale };
    scale = 1.f;
    const SkIPoint prevPos { rect.topLeft() };
    const SkISize prevSize { rect.size() };
    rect.offsetTo(0, 0);
    updateRect();
    painter->bindFramebuffer(&fb);
    painter->enableCustomTextureColor(false);
    painter->enableAutoBlendFunc(true);
    painter->setAlpha(1.f);
    painter->setColorFactor(1.f, 1.f, 1.f, 1.f);
    fractionalFb.setFence();
    painter->bindTextureMode({
        .texture = fractionalFb.texture(0),
        .pos = rect.topLeft(),
        .srcRect = SkRect::MakeWH(fractionalFb.sizeB().width(), fractionalFb.sizeB().height()),
        .dstSize = rect.size(),
        .srcTransform = CZTransform::Normal,
        .srcScale = 1.f
    });

    glDisable(GL_BLEND);

    if (cursorOnly)
    {
        SkRegion cursorDamage { prevCursorRect };
        CZRegionUtils::ApplyTransform(cursorDamage, prevSize, prevTrasform);
        SkRegion tmp;

        SkRegion::Iterator it(cursorDamage);

        while (!it.done())
        {
            tmp.op(
                SkIRect::MakeXYWH(
                    SkScalarFloorToInt(Float32(it.rect().x()) * prevFracScale) - 2,
                    SkScalarFloorToInt(Float32(it.rect().y()) * prevFracScale) - 2,
                    SkScalarCeilToInt(Float32(it.rect().width()) * prevFracScale) + 4,
                    SkScalarCeilToInt(Float32(it.rect().height()) * prevFracScale) + 4),
                SkRegion::kUnion_Op);
            it.next();
        }

        cursorDamage.op(
            SkIRect::MakeSize(output->currentMode()->sizeB()),
            std::move(tmp),
            SkRegion::kIntersect_Op);

        painter->drawRegion(cursorDamage);
    }
    else
        painter->drawRegion(damage);

    stateFlags.add(UsingFractionalScale);
    transform = prevTrasform;
    scale = prevScale;
    rect.offsetTo(prevPos.x(), prevPos.y());
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
    if (stateFlags.hasAll(UsingFractionalScale | FractionalOversamplingEnabled))
    {
        if (stateFlags.hasAll(ScreenshotsWithCursor | ScreenshotsWithoutCursor))
        {
            blitFractionalScaleFb(false);
            handleScreenshotRequests(false);
            painter->bindFramebuffer(&fractionalFb);
            drawCursor();
            blitFractionalScaleFb(true);
            handleScreenshotRequests(true);
        }
        else if (stateFlags.has(ScreenshotsWithCursor) && !stateFlags.has(ScreenshotsWithoutCursor))
        {
            drawCursor();
            blitFractionalScaleFb(false);
            handleScreenshotRequests(true);
        }
        else if (!stateFlags.has(ScreenshotsWithCursor) && stateFlags.has(ScreenshotsWithoutCursor))
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
        if (stateFlags.hasAll(ScreenshotsWithCursor | ScreenshotsWithoutCursor))
        {
            handleScreenshotRequests(false);
            painter->bindFramebuffer(&fb);
            drawCursor();
            handleScreenshotRequests(true);
        }
        else if (stateFlags.has(ScreenshotsWithCursor) && !stateFlags.has(ScreenshotsWithoutCursor))
        {
            drawCursor();
            handleScreenshotRequests(true);
        }
        else if (!stateFlags.has(ScreenshotsWithCursor) && stateFlags.has(ScreenshotsWithoutCursor))
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

    if (lastPos != rect.topLeft())
    {
        output->moveGL();
        lastPos = rect.topLeft();
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
    damage.setRect(output->rect());

    /* Add the region the user should repaint if hw cursor
     * is disabled */
    calculateCursorDamage();

    const bool needsFullRepaintPrev { stateFlags.has(NeedsFullRepaint) };

    /* If the last frame had a custom scanout buffer and the current
     * one doesn't, suggest a full repaint */
    if (stateFlags.has(HasScanoutBuffer))
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

    if (!stateFlags.has(HasScanoutBuffer))
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

    if (lastPos != rect.topLeft())
    {
        output->moveGL();
        lastPos = rect.topLeft();
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
    if (stateFlags.has(UsingFractionalScale))
    {
        sizeB = compositor()->imp()->graphicBackend->outputGetCurrentMode(output)->sizeB();
        sizeB.fWidth = SkScalarRoundToInt(Float32(sizeB.width()) * scale / fractionalScale);
        sizeB.fHeight = SkScalarRoundToInt(Float32(sizeB.height()) * scale / fractionalScale);
    }
    else
        sizeB = compositor()->imp()->graphicBackend->outputGetCurrentMode(output)->sizeB();

    // Swap width with height
    if (CZ::Is90Transform(transform))
    {
        const Int32 tmpW { sizeB.width() };
        sizeB.fWidth = sizeB.height();
        sizeB.fHeight = tmpW;
    }

    rect.setXYWH(
        rect.x(), rect.y(),
        roundf(Float32(sizeB.width())/scale),
        roundf(Float32(sizeB.height())/scale));

    if (!stateFlags.has(IsBlittingFramebuffers))
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
    cursorDamage.setEmpty();

    if (cursor()->enabled(output) && !cursor()->hwCompositingEnabled(output) && cursor()->visible())
    {
        stateFlags.add(CursorNeedsRendering);
        cursorDamage.op(cursor()->rect(), SkRegion::Op::kUnion_Op);
        cursorDamage.op(SkIRect::MakePtSize(prevCursorRect.topLeft() + output->pos(), prevCursorRect.size()), SkRegion::kUnion_Op);
        cursorDamage.op(output->rect(), SkRegion::kIntersect_Op);
        dirtyCursorFBs = output->buffersCount();
    }

    if (stateFlags.has(CursorRenderedInPrevFrame) && cursor()->hwCompositingEnabled(output))
    {
        cursorDamage.op(SkIRect::MakePtSize(prevCursorRect.topLeft() + output->pos(), prevCursorRect.size()), SkRegion::kUnion_Op);
        output->repaint();

        if (dirtyCursorFBs > 0)
            dirtyCursorFBs--;
        else
            stateFlags.remove(CursorRenderedInPrevFrame);
    }

    prevCursorRect = SkIRect::MakePtSize(cursor()->rect().topLeft() - output->pos(), cursor()->rect().size());
}

void LOutput::LOutputPrivate::drawCursor() noexcept
{
    // Manualy draw the cursor if hardware composition is not supported
    if (stateFlags.has(CursorNeedsRendering))
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
            .pos = cursor()->rect().topLeft(),
            .srcRect = SkRect::MakeWH(
                 cursor()->texture()->sizeB().width(),
                 cursor()->texture()->sizeB().height()),
            .dstSize = cursor()->rect().size(),
            .srcTransform = CZTransform::Normal,
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
        if (stateFlags.has(HasScanoutBuffer) || // Deny screenshoots when there is a custom scanout buffer
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

    if (stateFlags.has(ScreenshotsWithCursor))
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
    SkIRect prev { 0, 0, 0, 0 };

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
            zone->m_rect.fLeft = exclusiveEdges.left;
            zone->m_rect.fTop = exclusiveEdges.top;
            zone->m_rect.fRight = zone->m_rect.fLeft + zone->size();
            zone->m_rect.fBottom = rect.height() - exclusiveEdges.bottom;
            exclusiveEdges.left += zone->size();
            break;
        case LEdgeTop:
            zone->m_rect.fLeft = exclusiveEdges.left;
            zone->m_rect.fTop = exclusiveEdges.top;
            zone->m_rect.fBottom = zone->m_rect.fTop + zone->size();
            zone->m_rect.fRight = rect.width() - exclusiveEdges.right;
            exclusiveEdges.top += zone->size();
            break;
        case LEdgeRight:
            zone->m_rect.fTop = exclusiveEdges.top;
            zone->m_rect.fBottom = rect.height() - exclusiveEdges.bottom;
            exclusiveEdges.right += zone->size();
            zone->m_rect.fLeft = rect.width() - exclusiveEdges.right;
            zone->m_rect.fRight = zone->m_rect.fLeft + zone->size();
            break;
        case LEdgeBottom:
            zone->m_rect.fLeft = exclusiveEdges.left;
            zone->m_rect.fRight = rect.width() - exclusiveEdges.right;
            exclusiveEdges.bottom += zone->size();
            zone->m_rect.fTop = rect.height() - exclusiveEdges.bottom;
            zone->m_rect.fBottom = zone->m_rect.fTop + zone->size();
            break;
        }

        if (zone->m_onRectChangeCallback && prev != zone->m_rect)
            zone->m_onRectChangeCallback(zone);
    }

    prev = availableGeometry;
    availableGeometry.fLeft = exclusiveEdges.left;
    availableGeometry.fTop = exclusiveEdges.top;
    availableGeometry.fRight = rect.width() - exclusiveEdges.right;
    availableGeometry.fBottom = rect.height() - exclusiveEdges.bottom;

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
                zone->m_rect.setSize(rect.size());
        }
        else
        {
            if (zone->size() == 0)
                zone->m_rect = availableGeometry;
            else if ( zone->size() < 0)
                zone->m_rect.setXYWH(
                    zone->m_rect.x(),
                    zone->m_rect.y(),
                    rect.width(),
                    rect.height());
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
