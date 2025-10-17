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

#include <CZ/Ream/RCore.h>
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
    initPromise.set_value(true);
}

void LOutput::LOutputPrivate::backendPaintGL()
{
    const auto lock { LLockGuard() };

    if (output->imp()->state != LOutput::Initialized)
        return;

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

    // Update active LAnimations
    compositor()->imp()->core->updateAnimations();

    compositor()->imp()->currentOutput = output;

    /* Mark the entire output rect as damaged for compositors
     * that do not track damage.*/
    output->damage.setRect(SkIRect::MakeSize(output->size()));

    const bool needsFullRepaintPrev { stateFlags.has(NeedsFullRepaint) };

    /* Let users do their rendering*/
    stateFlags.add(IsInPaintGL);
    resizeOSSurface();
    output->paintGL();
    stateFlags.remove(IsInPaintGL);

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

    /* Ensure clients receive frame callbacks and pending roles configurations on time */
    compositor()->flushClients();

    /* Destroy render buffers created from this thread and marked as destroyed by the user */
    RCore::Get()->clearGarbage();
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
    std::lock_guard<std::mutex> lock { compositor()->imp()->presentationMutex };
    CZPresentationEvent e {};
    e.info = info;
    e.discarded = false;
    presentationEventQueue.emplace(e);
}

void LOutput::LOutputPrivate::backendDiscarded(UInt64 paintEventId) noexcept
{
    std::lock_guard<std::mutex> lock { compositor()->imp()->presentationMutex };
    CZPresentationEvent e {};
    e.info.paintEventId = paintEventId;
    e.discarded = true;
    presentationEventQueue.emplace(e);
}

void LOutput::LOutputPrivate::damageToBufferCoords() noexcept
{
    CZRegionUtils::ApplyTransform(output->damage, rect.size(), transform);

    SkRegion tmp;
    SkRegion::Iterator it { output->damage };

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

    output->damage.op(SkIRect::MakeSize(output->currentMode()->size()), tmp, SkRegion::kIntersect_Op);
    output->backend()->setDamage(output->damage);
}

void LOutput::LOutputPrivate::blitFractionalScaleFb(bool /*cursorOnly*/) noexcept
{

    /* TODO
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


    if (cursorOnly)
    {
        //painter->drawRegion(cursorDamage);
    }
    //else
    //    painter->drawRegion(damage);

    stateFlags.add(UsingFractionalScale);
    transform = prevTrasform;
    scale = prevScale;
    rect.offsetTo(prevPos.x(), prevPos.y());
    updateRect();
    */
}

void LOutput::LOutputPrivate::blitFramebuffers() noexcept
{

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

        if (osSurface)
            osSurface->resize(fbSize, 1, true);
        else
            osSurface = RSurface::Make(fbSize, 1.f, false);
    }
    else
        osSurface.reset();
}
