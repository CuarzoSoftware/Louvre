#include <protocols/Wayland/GOutput.h>
#include <protocols/ScreenCopy/RScreenCopyFrame.h>
#include <protocols/ScreenCopy/GScreenCopyManager.h>
#include <protocols/SessionLock/RSessionLock.h>
#include <private/LOutputPrivate.h>
#include <private/LOutputModePrivate.h>
#include <private/LCompositorPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LCursorPrivate.h>
#include <private/LSurfacePrivate.h>
#include <private/LClientPrivate.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <LExclusiveZone.h>
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
    if (output->gammaSize() != 0)
        output->setGamma(nullptr);

    threadId = std::this_thread::get_id();

    painter = new LPainter();
    painter->imp()->output = output;
    painter->bindFramebuffer(output->framebuffer());

    output->imp()->global.reset(compositor()->createGlobal<Protocols::Wayland::GOutput>(output));

    output->setScale(output->imp()->fractionalScale);
    lastPos = rect.pos();
    lastSize = rect.size();
    cursor()->imp()->textureChanged = true;
    cursor()->imp()->update();
    output->imp()->state = LOutput::Initialized;

    if (sessionLockRole && sessionLockRole->surface())
        sessionLockRole->surface()->imp()->setMapped(true);

    output->initializeGL();
    compositor()->flushClients();
}

void LOutput::LOutputPrivate::damageToBufferCoords() noexcept
{
    cursor()->enableHwCompositing(output, screenshotCursorTimeout == 0);

    if (screenshotCursorTimeout > 0 && cursor()->visible() && cursor()->enabled(output))
    {
        damage.addRect(prevCursorRect);
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
        cursorDamage.offset(-prevPos.x(), -prevPos.y());
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

    if (compositor()->imp()->runningAnimations() && seat()->enabled())
        compositor()->imp()->unlockPoll();

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

    compositor()->imp()->sendPresentationTime();
    compositor()->imp()->processAnimations();
    stateFlags.remove(PendingRepaint);
    painter->bindFramebuffer(&fb);

    compositor()->imp()->currentOutput = output;

    if (seat()->enabled() && output->screenshotRequests().empty())
        wl_event_loop_dispatch(compositor()->imp()->waylandEventLoop, 0);

    damage.clear();
    damage.addRect(output->rect());
    calculateCursorDamage();
    output->paintGL();
    removeFromSessionLockPendingRepaint();
    validateScreenshotRequests();
    compositor()->imp()->currentOutput = nullptr;
    damageToBufferCoords();
    blitFramebuffers();

    compositor()->flushClients();
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

    if (sessionLockRole && sessionLockRole->surface())
       sessionLockRole->surface()->imp()->setMapped(false);

    while (!screenshotRequests.empty())
    {
       screenshotRequests.back()->resource().failed();
       screenshotRequests.pop_back();
    }

    output->uninitializeGL();
    removeFromSessionLockPendingRepaint();
    compositor()->flushClients();
    output->imp()->state = LOutput::Uninitialized;
    compositor()->imp()->destroyPendingRenderBuffers(&output->imp()->threadId);

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
    }

    if (stateFlags.check(CursorRenderedInPrevFrame))
    {
        stateFlags.remove(CursorRenderedInPrevFrame);
        cursorDamage.addRect(prevCursorRect);
    }

    prevCursorRect = cursor()->rect();
    cursorDamage.clip(output->rect());
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
        if (!screenshotRequests[i]->resource().accepted() ||
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

        if (zone->m_layerRole)
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

        if (zone->m_layerRole && prev != zone->m_rect)
            zone->m_layerRole->configureRequest();
    }

    availableGeometry.setX(exclusiveEdges.left);
    availableGeometry.setY(exclusiveEdges.top);
    availableGeometry.setW(rect.w() - exclusiveEdges.left - exclusiveEdges.right);
    availableGeometry.setH(rect.h() - exclusiveEdges.top - exclusiveEdges.bottom);

    for (LExclusiveZone *zone : exclusiveZones)
    {
        if (zone->m_layerRole)
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

        if (zone->m_layerRole && prev != zone->m_rect)
            zone->m_layerRole->configureRequest();
    }
}
