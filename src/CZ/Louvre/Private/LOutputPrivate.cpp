#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Protocols/SessionLock/RSessionLock.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Louvre/Private/LClientPrivate.h>
#include <CZ/Louvre/Private/LLockGuard.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Layout/LExclusiveZone.h>
#include <CZ/Louvre/Seat/LOutputMode.h>
#include <CZ/Louvre/Roles/LLayerRole.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/LGlobal.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Core/CZTime.h>
#include <CZ/Core/CZCore.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Private/LFreezeWatchdog.h>

#include <CZ/Ream/RCore.h>
#include <CZ/Ream/RPass.h>
#include <CZ/Ream/RPainter.h>
#include <CZ/Core/Utils/CZRegionUtils.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::Wayland;

void LOutput::LOutputPrivate::backendInitializeGL()
{
    const auto lock { LLockGuard() };
    threadId = std::this_thread::get_id();
    compositor()->imp()->initThreadData(output);
    compositor()->imp()->disablePendingPosixSignals();
    output->imp()->global.reset(compositor()->createGlobal<Protocols::Wayland::GOutput>(0, output));
    output->setScale(output->imp()->fractionalScale);
    lastPos = rect.topLeft();
    lastSize = rect.size();
    cursor()->updateLater();
    output->imp()->state = LOutput::Initialized;

    if (sessionLockRole && sessionLockRole->surface())
        sessionLockRole->surface()->imp()->setMapped(true);

    updateLayerSurfacesMapping();

    output->initializeGL();
    compositor()->flushClients();

    if (auto *watchdog { LFreezeWatchdog::Get() })
        watchdog->watch(output);

    initPromise.set_value(true);
}

void LOutput::LOutputPrivate::backendPaintGL()
{
    /* Frame latency instrumentation: the request time (0 if the backend repainted on its own),
     * the wake-up time (before the lock, so a stall acquiring it is measured) and the lock time. */
    FrameTiming timing {};
    timing.requestUs = repaintRequestUs.exchange(0);
    timing.wakeUs = CZTime::Us();
    paintSinceUs.store(timing.requestUs == 0 ? timing.wakeUs : timing.requestUs);

    const auto lock { LLockGuard() };

    timing.lockUs = CZTime::Us();

    if (output->imp()->state != LOutput::Initialized)
    {
        paintSinceUs.store(0);
        return;
    }

    compositor()->imp()->dispatchPresentationTimeEvents();
    compositor()->imp()->disablePendingPosixSignals();

    stateFlags.remove(PendingRepaint);

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


    if (compositor()->imp()->core->animationInterval() > 0)
        compositor()->imp()->core->updateAnimations();

    compositor()->imp()->currentOutput = output;

    /* Mark the entire output rect as damaged for compositors
     * that do not track damage.*/
    output->damage.setRect(SkIRect::MakeSize(output->size()));

    const bool needsFullRepaintPrev { stateFlags.has(NeedsFullRepaint) };

    /* Let users do their rendering*/
    stateFlags.add(IsInPaintGL);
    resizeOSSurface();
    updateSoftwareCursor();
    output->paintGL();
    stateFlags.remove(IsInPaintGL);

    /* The software cursor's previous and current rects are part of what changed on screen */
    if (!swCursorPrevRect.isEmpty())
        output->damage.op(swCursorPrevRect, SkRegion::kUnion_Op);

    if (!swCursorRect.isEmpty())
        output->damage.op(swCursorRect, SkRegion::kUnion_Op);

    handleUnpresentedSurfaces();

    stateFlags.setFlag(NeedsFullRepaint, needsFullRepaintPrev);

    /* This ensures that all active outputs have been repainted at least once after a client requests to lock the session.
     * Protocol quote: The locked event must not be sent until a new "locked" frame has been presented on all outputs and no
     * security sensitive normal/unlocked content is possibly visible. */
    removeFromSessionLockPendingRepaint();

    compositor()->imp()->currentOutput = nullptr;

    /* Turn damage into buffer coords and handle buffer
     * blitting if oversampling is enabled or there are
     * screen copy requests*/
    stateFlags.add(IsBlittingFramebuffers);
    damageToBufferCoords();
    blitFramebuffers();
    stateFlags.remove(IsBlittingFramebuffers);
    swCursorPrevRect = swCursorRect;

    /* Ensure clients receive frame callbacks and pending roles configurations on time */
    compositor()->flushClients();

    /* Destroy render buffers created from this thread and marked as destroyed by the user */
    RCore::Get()->clearGarbage();

    /* Frame latency instrumentation: store the timings, retrieved when presented/discarded */
    timing.endUs = CZTime::Us();
    paintSinceUs.store(0);
    {
        std::lock_guard<std::mutex> timingsLock { frameTimingsMutex };

        // Each frame is always presented or discarded, this should never grow
        if (frameTimings.size() > 16)
            frameTimings.clear();

        frameTimings[output->backend()->paintEventId()] = timing;
    }
}

void LOutput::LOutputPrivate::backendResizeGL() noexcept
{
    const auto lock { LLockGuard() };

    output->setScale(output->fractionalScale());
    output->imp()->updateRect();
    output->imp()->updateGlobals();
    cursor()->m_imageChanged = true;

    compositor()->imp()->disablePendingPosixSignals();

    output->resizeGL();

    if (lastPos != rect.topLeft())
    {
        output->moveGL();
        lastPos = rect.topLeft();
    }
}

void LOutput::LOutputPrivate::backendUninitializeGL()
{
    if (auto *watchdog { LFreezeWatchdog::Get() })
        watchdog->unwatch(output);

    repaintRequestUs.store(0);
    paintSinceUs.store(0);

    const auto lock { LLockGuard() };

    compositor()->imp()->disablePendingPosixSignals();

    if (sessionLockRole && sessionLockRole->surface())
        sessionLockRole->surface()->imp()->setMapped(false);

    output->uninitializeGL();
    removeFromSessionLockPendingRepaint();

    compositor()->flushClients();
    output->imp()->state = LOutput::Uninitialized;
    updateLayerSurfacesMapping();
    compositor()->imp()->unitThreadData();
    unitPromise.set_value(true);
}

void LOutput::LOutputPrivate::backendPresented(const CZPresentationTime &info) noexcept
{
    logFrameTiming(info.paintEventId, false);
    std::lock_guard<std::mutex> lock { compositor()->imp()->presentationMutex };
    CZPresentationEvent e {};
    e.info = info;
    e.discarded = false;
    presentationEventQueue.emplace(e);
}

void LOutput::LOutputPrivate::backendDiscarded(UInt64 paintEventId) noexcept
{
    logFrameTiming(paintEventId, true);
    std::lock_guard<std::mutex> lock { compositor()->imp()->presentationMutex };
    CZPresentationEvent e {};
    e.info.paintEventId = paintEventId;
    e.discarded = true;
    presentationEventQueue.emplace(e);
}

void LOutput::LOutputPrivate::logFrameTiming(UInt64 paintEventId, bool discarded) noexcept
{
    FrameTiming timing;

    {
        std::lock_guard<std::mutex> lock { frameTimingsMutex };
        const auto it { frameTimings.find(paintEventId) };

        if (it == frameTimings.end())
            return;

        timing = it->second;
        frameTimings.erase(it);
    }

    const UInt64 nowUs { CZTime::Us() };
    const UInt64 fromUs { timing.requestUs == 0 ? timing.wakeUs : timing.requestUs };
    const Float64 totalMs { Float64(nowUs - fromUs) / 1000.0 };
    const Float64 wakeMs { timing.requestUs == 0 ? 0.0 : Float64(timing.wakeUs - timing.requestUs) / 1000.0 };
    const Float64 lockMs { Float64(timing.lockUs - timing.wakeUs) / 1000.0 };
    const Float64 paintMs { Float64(timing.endUs - timing.lockUs) / 1000.0 };
    const Float64 flipMs { Float64(nowUs - timing.endUs) / 1000.0 };

    /* A frame normally takes up to ~2 refresh cycles from request to presentation */
    constexpr Float64 warnThresholdMs { 50.0 };

    LLog(totalMs >= warnThresholdMs ? CZWarning : CZDebug, CZLN,
         "[{}] Frame {} in {:.2f} ms (request -> wake {:.2f} ms, wake -> lock {:.2f} ms, paint {:.2f} ms, {} wait {:.2f} ms)",
         output->name(), discarded ? "DISCARDED" : "presented", totalMs,
         wakeMs, lockMs, paintMs, discarded ? "discard" : "flip", flipMs);
}

SkRegion LOutput::LOutputPrivate::localToBuffer(const SkRegion &region) const noexcept
{
    SkRegion transformed { region };
    CZRegionUtils::ApplyTransform(transformed, rect.size(), transform);

    // Outset by 2 px: linear filtering (fractional scales) samples neighbor pixels
    SkRegion result;
    SkRegion::Iterator it { transformed };

    while (!it.done())
    {
        result.op(
            SkIRect::MakeXYWH(
            SkScalarFloorToInt(Float32(it.rect().x()) * fractionalScale) - 2,
            SkScalarFloorToInt(Float32(it.rect().y()) * fractionalScale) - 2,
            SkScalarCeilToInt(Float32(it.rect().width()) * fractionalScale) + 4,
            SkScalarCeilToInt(Float32(it.rect().height()) * fractionalScale) + 4),
            SkRegion::kUnion_Op);
        it.next();
    }

    result.op(SkIRect::MakeSize(output->currentMode()->size()), SkRegion::kIntersect_Op);
    return result;
}

void LOutput::LOutputPrivate::damageToBufferCoords() noexcept
{
    output->damage = localToBuffer(output->damage);
    output->backend()->setDamage(output->damage);
}

void LOutput::LOutputPrivate::blitFractionalScaleFb() noexcept
{
    auto target { output->backendImage() };

    if (!target || !osSurface || !osSurface->image())
        return;

    /* The screen images rotate: the one painted now was last painted `age` frames ago, so it also
     * misses what changed in the frames in between (osSurface has it all). */
    const SkRegion &frameDamage { output->damage }; // Buffer coords (see damageToBufferCoords())
    const UInt32 age { output->backend()->imageAge() };
    const SkIRect full { SkIRect::MakeSize(target->size()) };
    SkRegion blit;

    if (age == 0 || age > osDamageRing.size() + 1)
        blit.setRect(full);
    else
    {
        blit = frameDamage;

        for (UInt32 i = 0; i + 1 < age; i++)
            blit.op(osDamageRing[i], SkRegion::kUnion_Op);
    }

    osDamageRing.push_front(frameDamage);

    while (osDamageRing.size() > 8)
        osDamageRing.pop_back();

    if (blit.isEmpty())
        return;

    // Pixel to pixel: osSurface is already transformed, just larger (ceil(scale) / fractional scale)
    auto surface { RSurface::WrapImage(target) };

    if (!surface)
        return;

    surface->setGeometry({
        .viewport = SkRect::Make(full),
        .dst = SkRect::Make(full),
        .transform = CZTransform::Normal });

    auto pass { surface->beginPass(RPassCap_Painter) };

    if (!pass)
        return;

    auto *painter { pass->getPainter() };
    painter->setBlendMode(RBlendMode::Src);
    painter->setOpacity(1.f);

    RDrawImageInfo info {};
    info.image = osSurface->image();
    info.src = osSurface->geometry().dst;
    info.srcScale = 1.f;
    info.srcTransform = CZTransform::Normal;
    info.dst = full;
    info.minFilter = RImageFilter::Linear;
    info.magFilter = RImageFilter::Linear;
    painter->drawImage(info, &blit);
}

void LOutput::LOutputPrivate::blitFramebuffers() noexcept
{
    if (isOversampling())
        blitFractionalScaleFb();

    drawSoftwareCursor();
}

void LOutput::LOutputPrivate::updateSoftwareCursor() noexcept
{
    // Where the cursor is painted this frame, if Louvre paints it on this output
    swCursorRect.setEmpty();

    if (cursor()->isVisible() && !cursor()->isPlaneEnabled(output) && cursor()->m_image)
    {
        SkIRect r { cursor()->rect() };
        r.offset(-rect.x(), -rect.y());

        if (SkIRect::Intersects(r, SkIRect::MakeSize(rect.size())))
            swCursorRect = r;
    }

    // Without oversampling the compositor must repaint under the previous cursor (and where the new
    // one goes, so it's part of the damage). With oversampling, Louvre restores it from osSurface.
    swCursorDamage.setEmpty();

    if (!isOversampling())
    {
        if (!swCursorPrevRect.isEmpty())
            swCursorDamage.op(swCursorPrevRect, SkRegion::kUnion_Op);

        if (!swCursorRect.isEmpty())
            swCursorDamage.op(swCursorRect, SkRegion::kUnion_Op);
    }
}

void LOutput::LOutputPrivate::drawSoftwareCursor() noexcept
{
    auto image { cursor()->m_image };
    auto target { output->backendImage() };

    if (swCursorRect.isEmpty() || !image || !target)
        return;

    // Output-local surface coords mapped to the real screen pixels (transform and fractional scale)
    auto surface { RSurface::WrapImage(target) };

    if (!surface)
        return;

    surface->setGeometry({
        .viewport = SkRect::MakeWH(rect.width(), rect.height()),
        .dst = SkRect::Make(SkIRect::MakeSize(target->size())),
        .transform = transform });

    auto pass { surface->beginPass(RPassCap_Painter) };

    if (!pass)
        return;

    auto *painter { pass->getPainter() };
    painter->setBlendMode(RBlendMode::SrcOver);
    painter->setOpacity(1.f);

    RDrawImageInfo info {};
    info.image = image;
    info.src = SkRect::Make(image->size());
    info.srcScale = 1.f;
    info.srcTransform = CZTransform::Normal;
    info.dst = swCursorRect;
    info.minFilter = RImageFilter::Linear;
    info.magFilter = RImageFilter::Linear;

    const SkRegion clip { swCursorRect };
    painter->drawImage(info, &clip);
}

void LOutput::LOutputPrivate::updateRect()
{
    if (stateFlags.has(UsingFractionalScale))
    {
        bufferSize = output->currentMode()->size();
        bufferSize.fWidth = SkScalarRoundToInt(Float32(bufferSize.width()) * scale / fractionalScale);
        bufferSize.fHeight = SkScalarRoundToInt(Float32(bufferSize.height()) * scale / fractionalScale);
    }
    else
        bufferSize = output->currentMode()->size();

    // Swap width and height
    if (CZ::Is90Transform(transform))
    {
        const auto tmpWidth { bufferSize.width() };
        bufferSize.fWidth = bufferSize.height();
        bufferSize.fHeight = tmpWidth;
    }

    rect.setXYWH(
        rect.x(), rect.y(),
        roundf(Float32(bufferSize.width())/scale),
        roundf(Float32(bufferSize.height())/scale));

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

void LOutput::LOutputPrivate::removeFromSessionLockPendingRepaint() noexcept
{
    if (sessionLockManager()->state() == LSessionLockManager::Locked && !sessionLockManager()->m_sessionLockRes->m_lockedOnce)
    {
        CZVectorUtils::RemoveOneUnordered(sessionLockManager()->m_sessionLockRes->m_pendingRepaint, output);

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
        case CZEdgeNone:
            break;
        case CZEdgeLeft:
            zone->m_rect.fLeft = exclusiveEdges.left;
            zone->m_rect.fTop = exclusiveEdges.top;
            zone->m_rect.fRight = zone->m_rect.fLeft + zone->size();
            zone->m_rect.fBottom = rect.height() - exclusiveEdges.bottom;
            exclusiveEdges.left += zone->size();
            break;
        case CZEdgeTop:
            zone->m_rect.fLeft = exclusiveEdges.left;
            zone->m_rect.fTop = exclusiveEdges.top;
            zone->m_rect.fBottom = zone->m_rect.fTop + zone->size();
            zone->m_rect.fRight = rect.width() - exclusiveEdges.right;
            exclusiveEdges.top += zone->size();
            break;
        case CZEdgeRight:
            zone->m_rect.fTop = exclusiveEdges.top;
            zone->m_rect.fBottom = rect.height() - exclusiveEdges.bottom;
            exclusiveEdges.right += zone->size();
            zone->m_rect.fLeft = rect.width() - exclusiveEdges.right;
            zone->m_rect.fRight = zone->m_rect.fLeft + zone->size();
            break;
        case CZEdgeBottom:
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

        if (zone->edge() == CZEdgeNone)
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

void LOutput::LOutputPrivate::handleUnpresentedSurfaces() noexcept
{
    for (LSurface *s : compositor()->surfaces())
    {
        // We should not send discard if the surface doesn't intersect this output
        // It may be presented on another output soon
        if (!s->outputs().contains(output) && !s->outputs().empty())
            continue;

        while (!s->imp()->current.presentationFeedbackRes.empty())
        {
            if (s->imp()->current.presentationFeedbackRes.front())
                s->imp()->current.presentationFeedbackRes.front()->discarded();

            s->imp()->current.presentationFeedbackRes.pop_front();
        }
    }
}

void LOutput::LOutputPrivate::resizeOSSurface() noexcept
{
    if (stateFlags.hasAll(UsingFractionalScale | OversamplingEnabled))
    {
        auto fbSize { output->currentMode()->size() };
        fbSize = SkISize(
            SkScalarRoundToInt(Float32(fbSize.width()) * scale / fractionalScale),
            SkScalarRoundToInt(Float32(fbSize.height()) * scale / fractionalScale));
        fbSize.fWidth = fbSize.width() + fbSize.width() % (Int32)scale;
        fbSize.fHeight = fbSize.height() + fbSize.height() % (Int32)scale;

        bool fresh { true };

        if (osSurface)
            fresh = osSurface->resize(fbSize, 1, true);
        else
            osSurface = RSurface::Make(fbSize, 1.f, false);

        // A new/resized image has no content: fully repainted, and fully blitted (see blitFractionalScaleFb())
        osAge = fresh ? 0 : 1;

        if (fresh)
            osDamageRing.clear();
    }
    else
    {
        osSurface.reset();
        osAge = 0;
        osDamageRing.clear();
    }
}
