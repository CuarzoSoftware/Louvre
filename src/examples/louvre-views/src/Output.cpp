#include <LCursor.h>
#include <LAnimation.h>
#include <LLayerView.h>
#include <LSurfaceView.h>
#include <LPainter.h>
#include <LLog.h>
#include <LOpenGL.h>
#include <LTextureView.h>
#include <LOutputMode.h>
#include <LSeat.h>
#include <LPointerMoveEvent.h>
#include <LUtils.h>
#include <LScreenshotRequest.h>

#include "Global.h"
#include "Output.h"
#include "Compositor.h"
#include "Dock.h"
#include "LSessionLockRole.h"
#include "Topbar.h"
#include "Workspace.h"
#include "Toplevel.h"
#include "Surface.h"
#include "ToplevelView.h"
#include "LayerRole.h"

Output::Output(const void *params) noexcept : LOutput(params)
{
    zoomScene.enableParentOffset(false);
    zoomView.enableDstSize(true);
    zoomView.setTranslucentRegion(&LRegion::EmptyRegion());

    workspacesContainer.enableParentOffset(false);
    workspacesContainer.setPos(0, 0);

    wallpaper.enableDstSize(true);
    wallpaper.enableSrcRect(compositor()->graphicBackendId() == LGraphicBackendWayland);
    wallpaper.enablePointerEvents(true);
    wallpaper.enableBlockPointer(true);
    wallpaper.setTranslucentRegion(&LRegion::EmptyRegion());
    wallpaper.setUserData(WallpaperType);
}

void Output::initializeGL()
{
    workspacesContainer.setParent(&G::compositor()->workspacesLayer);
    currentWorkspace = new Workspace(this);
    topbar.initialize();
    dock.initialize();
    wallpaper.setParent(&G::compositor()->backgroundLayer);
    updateWallpaper();
    updateWorkspacesPos();
    G::compositor()->scene.handleInitializeGL(this);
}

void Output::resizeGL()
{
    G::arrangeOutputs(this);
    updateWorkspacesPos();
    setWorkspace(currentWorkspace, 1);
    topbar.updateOutputInfo();
    topbar.update();
    dock.update();
    updateWallpaper();
    G::compositor()->scene.handleResizeGL(this);
}

void Output::moveGL()
{
    updateWorkspacesPos();
    topbar.update();
    dock.update();
    updateWallpaper();
    setWorkspace(currentWorkspace, 1);
    G::compositor()->scene.handleMoveGL(this);
}

void Output::paintGL()
{
    // Show black screen during output removal
    if (!G::compositor()->checkUpdateOutputUnplug())
    {
        painter()->clearScreen();
        repaint();
        return;
    }

    // Check pointer events before painting
    if (G::compositor()->updatePointerBeforePaint)
    {
        seat()->pointer()->pointerMoveEvent(LPointerMoveEvent());
        G::compositor()->updatePointerBeforePaint = false;
    }

    updateFractionalOversampling();

    if (currentWorkspace->toplevel)
        setContentType(currentWorkspace->toplevel->surface()->contentType());
    else
        setContentType(LContentTypeNone);

    /* Do not render when setting custom scanout buffers */
    if (tryFullscreenScanoutIfNoOverlayContent())
        return;

    const bool zoomed { zoom < 1.f && cursor()->output() == this };

    if (zoomed)
        zoomedDrawBegin();
    else
        cursor()->enable(this, true);

    G::scene()->handlePaintGL(this);

    if (zoomed)
        zoomedDrawEnd();

    for (auto *screenshotRequest : screenshotRequests())
        screenshotRequest->accept(true);
}

void Output::uninitializeGL()
{
    G::compositor()->outputUnplugHandled = false;

    // Find another output
    Output *aliveOutput = nullptr;

    for (Output *o : G::outputs())
    {
        if (o != this)
        {
            aliveOutput = o;
            break;
        }
    }

    // Destroy all fullscreen toplevel workspaces
    while (workspaces.size() != 1)
    {
        Toplevel *tl = workspaces.back()->toplevel;
        tl->surf()->sendOutputEnterEvent(aliveOutput);
        tl->outputUnplugConfigureCount = 0;
        tl->prevStates = LToplevelRole::Activated;
        tl->prevRect.setPos(LPoint(0, TOPBAR_HEIGHT));
        tl->configureSize(tl->prevRect.size());
        tl->configureState(LToplevelRole::Activated);
        tl->quickUnfullscreen = true;
        tl->unsetFullscreen();
        tl->surf()->localOutputPos = tl->prevRect.pos() - pos();
        tl->surf()->localOutputSize = size();
        tl->surf()->outputUnplugHandled = false;
        workspacesAnimation.stop();
    }

    delete workspaces.front();
    workspacesContainer.setPos(0, 0);
    workspacesContainer.setParent(nullptr);

    for (Surface *s : G::surfaces())
    {
        if (s->cursorRole() || (s->toplevel() && s->toplevel()->fullscreen()))
            continue;

        Output *intersectedOutput = G::mostIntersectedOuput(s->getView());

        if (intersectedOutput == this)
        {
            s->localOutputPos = s->pos() - pos();
            s->localOutputSize = size();
            s->outputUnplugHandled = false;
        }
        else if (!intersectedOutput)
        {
            s->localOutputPos = LPoint(200, 200);
            s->localOutputSize = size();
            s->outputUnplugHandled = false;
        }

        if (s->minimizedOutput == this)
        {
            s->minimizeAnim.stop();
            s->minimizedOutput = aliveOutput;
            s->minimizeStartRect.setPos(LPoint(rand() % 128, TOPBAR_HEIGHT + (rand() % 128)));
        }
    }

    dock.uninitialize();
    topbar.uninitialize();
    wallpaper.setParent(nullptr);
    scaledWallpaper.reset();
    G::compositor()->scene.handleUninitializeGL(this);
}

void Output::setWorkspace(Workspace *ws, UInt32 animMs, Float64 curve, Float64 start)
{
    animStartOffset = start;
    animEasingCurve = curve;
    workspacesAnimation.stop();
    workspacesAnimation.setDuration(animMs * DEBUG_ANIM_SPEED);
    currentWorkspace = ws;

    if (ws && currentWorkspace->toplevel && currentWorkspace->toplevel->surf())
    {
        seat()->keyboard()->setFocus(currentWorkspace->toplevel->surface());
        currentWorkspace->toplevel->configureState(currentWorkspace->toplevel->pendingConfiguration().state | LToplevelRole::Activated);
        enableVSync(currentWorkspace->toplevel->surf()->preferVSync());
    }
    else
    {
        bool foundSurface { false };

        for (auto it = compositor()->layer(LSurfaceLayer::LLayerMiddle).rbegin();
             it != compositor()->layer(LSurfaceLayer::LLayerMiddle).rend(); it++)
        {
            if ((*it)->toplevel() && (*it)->mapped() && !(*it)->minimized())
            {
                foundSurface = true;
                (*it)->toplevel()->configureState((*it)->toplevel()->pendingConfiguration().state | LToplevelRole::Activated);
                seat()->keyboard()->setFocus((*it));
                break;
            }
        }

        if (!foundSurface)
            seat()->keyboard()->setFocus(nullptr);

        enableVSync(true);
    }

    topbar.update();

    for (Output *o : G::outputs())
        o->workspaces.front()->stealChildren();

    workspaceAnimationInFirstFrame = true;
    workspacesAnimation.start();
    onWorkspacesAnimationUpdate(&workspacesAnimation);
}

void Output::updateWorkspacesPos()
{
    Int32 offset = 0;
    Int32 spacing = 128;

    for (Workspace *ws : workspaces)
    {
        if (ws->nativePos().x() != offset)
        {
            ws->setPos(offset, 0);

            if (ws->toplevel)
            {
                ws->toplevel->configureSize(size());
                ws->toplevel->configureState(ws->toplevel->pendingConfiguration().state);
            }
        }

        offset += size().w() + spacing;
    }
}

void Output::updateFractionalOversampling()
{
    bool oversampling { dpi() < 200 };
    bool fullscreenOrSubsurface { false };

    if (!usingFractionalScale() || doingFingerWorkspaceSwipe || workspacesAnimation.running())
        goto checkChange;

    if (currentWorkspace->toplevel)
    {
        if (currentWorkspace->toplevel->decoratedView && currentWorkspace->toplevel->decoratedView->fullscreenTopbarVisibility != 0.f)
        {
            fullscreenOrSubsurface = true;
            goto checkChange;
        }

        for (LSurface *surf : currentWorkspace->toplevel->surf()->children())
        {
            if (surf->toplevel() && surf->toplevel()->decorationMode() == LToplevelRole::ServerSide && surf->mapped() && !surf->minimized())
            {
                fullscreenOrSubsurface = true;
                goto checkChange;
            }
        }
    }
    else
    {
        for (Surface *surf : G::surfaces())
        {
            if (!surf->mapped() || surf->minimized())
                continue;

            if (surf->subsurface())
            {
                for (LOutput *o : surf->getView()->outputs())
                {
                    if (o == this)
                    {
                        fullscreenOrSubsurface = true;
                        goto checkChange;
                    }
                }
            }
            else if (surf->toplevel())
            {
                if (surf->tl()->decoratedView)
                {
                    for (LOutput *o : surf->getView()->outputs())
                    {
                        if (o == this)
                        {
                            fullscreenOrSubsurface = true;
                            goto checkChange;
                        }
                    }
                }
            }
        }
    }

    checkChange:

    oversampling = oversampling || fullscreenOrSubsurface;

    if (oversampling != fractionalOversamplingEnabled())
    {
        enableFractionalOversampling(oversampling);
        topbar.update();
    }
}

static bool hasMappedChildren(LSurface *surface)
{
    if (!surface)
        return false;

    for (LSurface *child : surface->children())
        if (child->mapped())
            return true;

    return false;
}

bool Output::tryFullscreenScanoutIfNoOverlayContent() noexcept
{
    LSurface *fullscreenSurface { nullptr };

    /* Try with a sessionLock surface or fullscreen toplevel */
    if (sessionLockRole() && sessionLockRole()->surface()->mapped())
        fullscreenSurface = sessionLockRole()->surface();
    else if (currentWorkspace->toplevel
             && currentWorkspace->toplevel->surface()->mapped()
             && (!currentWorkspace->toplevel->decoratedView
                 || currentWorkspace->toplevel->decoratedView->fullscreenTopbarVisibility == 0.f))
        fullscreenSurface = currentWorkspace->toplevel->surface();

    if (!fullscreenSurface
        || fullscreenSurface->bufferTransform() != transform()
        || (cursor()->visible() && !cursor()->hwCompositingEnabled(this))
        || !screenshotRequests().empty()
        || doingFingerWorkspaceSwipe
        || workspacesAnimation.running()
        || dock.visiblePercent != 0.f
        || hasMappedChildren(fullscreenSurface)
        || zoom != 1.f
        || (shelf && shelf->surface()->size().h() + shelf->margins().bottom > 4))
        return false;

    const bool ret { setCustomScanoutBuffer(fullscreenSurface->texture()) };

    if (ret)
        fullscreenSurface->requestNextFrame(true);

    return ret;
}

void Output::showAllWorkspaces()
{
    for (auto *ws : workspaces)
        ws->show(true);
}

void Output::hideAllWorkspacesExceptCurrent()
{
    for (auto *ws : workspaces)
        if (ws != currentWorkspace)
            ws->show(false);
}

static bool toplevelOrSubsurfacesHaveNewDamage(Surface *surface) noexcept
{
    bool damaged { false };

    for (LSurface *child : surface->children())
        if (child->subsurface())
            damaged = damaged || toplevelOrSubsurfacesHaveNewDamage((Surface*)child);

    if (surface->damageId() != surface->prevDamageId)
    {
        surface->requestNextFrame(false);
        surface->prevDamageId = surface->damageId();
        return true;
    }

    return damaged;
}

void Output::onWorkspacesAnimationUpdate(LAnimation *anim) noexcept
{
    repaint();
    showAllWorkspaces();

    if (doingFingerWorkspaceSwipe)
    {
        anim->stop();
        return;
    }

    // Hide non visible workspaces
    for (Workspace *ws : workspaces)
        ws->setVisible(LRect(ws->pos() + pos(), size()).intersects(rect()));

    Float64 linear { anim->value() };

    // Early stop
    if (linear >= 0.85)
        linear = 1.0;

    Float64 ease { 1.0 - pow(animStartOffset + (1.0 - animStartOffset) * linear, animEasingCurve) };
    workspacesPosX = workspacesPosX * ease + Float64( - currentWorkspace->nativePos().x()) * (1.0 - ease);
    workspacesContainer.setPos(workspacesPosX, 0);

    for (Output *o : G::outputs())
        for (Workspace *workspace : o->workspaces)
            workspace->clipChildren();

    if (animatedFullscreenToplevel)
    {
        Toplevel *tl = animatedFullscreenToplevel;

        if (tl->destructorCalled || tl->quickUnfullscreen)
        {
            anim->stop();
            return;
        }

        // Current fullscreen size
        LSize cSize;

        if (tl->fullscreen())
        {
            /*
             * Quick Reminder:
             *
             * - captureView: Displays a capture of the toplevel and subsurfaces before going fullscreen.
             * - animScene: Blits the current state of the toplevel and subsurfaces into a texture (can change
             *              each frame, e.g., if it is a web browser displaying a video).
             * - animView: Displays the animScene result.
             */

            Float32 val = 1.f - pow(1.0 - linear, 4.0);
            Float32 inv = 1.f - val;
            tl->animView.enableSrcRect(false);
            tl->animView.setVisible(true);

            if (toplevelOrSubsurfacesHaveNewDamage(tl->surf()) || workspaceAnimationInFirstFrame)
                tl->animScene->render();

            tl->animView.setTexture(tl->animScene->texture());
            tl->animView.setPos((pos() * val) + (tl->prevBoundingRect.pos() * (inv)));
            cSize = (tl->fullscreenOutput->size() * val) + (tl->prevBoundingRect.size() * (inv));
            tl->animView.setDstSize(cSize);
            tl->animView.setOpacity(val);

            tl->captureView.setPos(tl->animView.pos());
            tl->captureView.setDstSize(cSize);
            LRegion transRegion = tl->captureTransRegion;
            LSizeF transRegionScale = LSizeF(cSize) / LSizeF(tl->prevBoundingRect.size());
            transRegion.multiply(transRegionScale.x(), transRegionScale.y());
            tl->captureView.setTranslucentRegion(&transRegion);
        }
        else
        {
            Float32 val = 1.f - pow(1.f - linear, 2.f);
            Float32 inv = 1.f - val;
            tl->animScene->setPos(pos());
            LPoint animPos = (pos() * inv) + (tl->prevBoundingRect.pos() * val);
            tl->surf()->setPos(0);
            LBox box = tl->surf()->getView()->boundingBox();
            LSize boxSize = LSize(box.x2 - box.x1, box.y2 - box.y1);
            cSize = (size() * inv) + (boxSize * val);

            tl->captureView.setOpacity(inv);
            tl->captureView.setPos(animPos);
            tl->captureView.setDstSize(cSize);

            if (tl->decoratedView)
                tl->surf()->setPos(LPoint() - (LPoint(box.x1, box.y1) - tl->animScene->nativePos()));
            else
                tl->surf()->setPos(tl->windowGeometry().pos());

            if (toplevelOrSubsurfacesHaveNewDamage(tl->surf()) || workspaceAnimationInFirstFrame)
                tl->animScene->render();

            LRegion transReg;
            transReg = *tl->animScene->translucentRegion();
            transReg.offset(LPoint() - tl->animScene->pos());
            tl->animView.setTexture(tl->animScene->texture());
            tl->animView.enableSrcRect(true);
            tl->animView.setSrcRect(LRectF(0, boxSize * tl->animScene->bufferScale()));
            tl->animView.enableDstSize(true);
            tl->animView.enableParentOffset(false);
            tl->animView.setPos(animPos);
            tl->animView.setDstSize(cSize);

            LSizeF regScale = LSizeF(cSize) / LSizeF(boxSize);
            transReg.multiply(regScale.x(), regScale.y());
            tl->animView.setTranslucentRegion(&transReg);

            tl->configureSize(tl->prevRect.size());
            tl->configureState(LToplevelRole::Activated);
        }

        if (tl->decoratedView)
            tl->decoratedView->updateGeometry();
    }

    workspaceAnimationInFirstFrame = false;

    if (linear == 1.0)
        anim->stop();
}

void Output::onWorkspacesAnimationFinish(LAnimation */*anim*/) noexcept
{
    hideAllWorkspacesExceptCurrent();

    if (currentWorkspace->toplevel)
    {
        Toplevel *tl = currentWorkspace->toplevel;
        tl->blackFullscreenBackground.setVisible(false);
        tl->captureTexture.reset();
        tl->animView.setTexture(nullptr);
        tl->animScene.reset();

        if (tl->destructorCalled || tl->quickUnfullscreen)
            goto returnChildren;

        seat()->pointer()->setFocus(tl->surface());
        seat()->keyboard()->setFocus(tl->surface());
        tl->configureState(tl->pendingConfiguration().state | LToplevelRole::Activated);
    }

    if (animatedFullscreenToplevel)
    {
        Toplevel *tl = animatedFullscreenToplevel;

        tl->blackFullscreenBackground.setVisible(false);
        tl->captureTexture.reset();
        tl->animView.setTexture(nullptr);
        tl->animScene.reset();

        if (tl->destructorCalled || tl->quickUnfullscreen)
            goto returnChildren;

        if (tl->fullscreen())
        {
            if (tl->decoratedView)
            {
                tl->decoratedView->decoT.setParent(&tl->fullscreenWorkspace->overlay);
                tl->decoratedView->topbarInput.setParent(&tl->fullscreenWorkspace->overlay);
                tl->decoratedView->buttonsContainer.setParent(&tl->fullscreenWorkspace->overlay);
            }

            tl->surf()->setPos(pos().x(), 0);
            G::reparentWithSubsurfaces(tl->surf(), &tl->fullscreenWorkspace->surfaces);
            currentWorkspace->clipChildren();
        }
        else
        {
            tl->surf()->setPos(tl->prevRect.pos());
            G::reparentWithSubsurfaces(tl->surf(), &workspaces.front()->surfaces, false);
            G::repositionNonVisibleToplevelChildren(this, tl->surf());
            tl->surf()->getView()->setVisible(true);
            tl->surf()->raise();
            delete tl->fullscreenWorkspace;
            tl->fullscreenWorkspace = nullptr;
        }

        animatedFullscreenToplevel = nullptr;

        if (tl->decoratedView)
            tl->decoratedView->updateGeometry();
    }

returnChildren:
    animatedFullscreenToplevel = nullptr;
    for (Output *o : G::outputs())
        if (!o->doingFingerWorkspaceSwipe && o->currentWorkspace)
            o->currentWorkspace->returnChildren();

    updateWorkspacesPos();
    G::scene()->mainView()->damageAll(this);
    repaint();
}

void Output::updateWallpaper() noexcept
{
    wallpaper.setPos(pos());
    wallpaper.setDstSize(size());

    if (!G::textures()->wallpaper || size().area() == 0)
        return;

    if (compositor()->graphicBackendId() == LGraphicBackendDRM)
    {
        LSize bufferSize;

        if (is90Transform(transform()))
        {
            bufferSize.setW(currentMode()->sizeB().h());
            bufferSize.setH(currentMode()->sizeB().w());
        }
        else
            bufferSize = currentMode()->sizeB();

        if (scaledWallpaper && scaledWallpaper->sizeB() == bufferSize)
            return;

        LRect srcB;
        const Float32 w { Float32(bufferSize.w() * G::textures()->wallpaper->sizeB().h()) / Float32(bufferSize.h()) };

        /* Clip and scale the wallpaper texture */

        if (w >= G::textures()->wallpaper->sizeB().w())
        {
            srcB.setX(0);
            srcB.setW(G::textures()->wallpaper->sizeB().w());
            srcB.setH((G::textures()->wallpaper->sizeB().w() * bufferSize.h()) / bufferSize.w());
            srcB.setY((G::textures()->wallpaper->sizeB().h() - srcB.h()) / 2);
        }
        else
        {
            srcB.setY(0);
            srcB.setH(G::textures()->wallpaper->sizeB().h());
            srcB.setW((G::textures()->wallpaper->sizeB().h() * bufferSize.w()) / bufferSize.h());
            srcB.setX((G::textures()->wallpaper->sizeB().w() - srcB.w()) / 2);
        }
        scaledWallpaper.reset(G::textures()->wallpaper->copy(bufferSize, srcB));
        wallpaper.setTexture(scaledWallpaper.get());
    }
    else
    {
        wallpaper.setTexture(G::textures()->wallpaper);
        const LSize &texSize { wallpaper.texture()->sizeB() };

        const Int32 outputScaledHeight { (texSize.w() * size().h())/size().w() };

        if (outputScaledHeight >= G::textures()->wallpaper->sizeB().h())
        {
            const Int32 outputScaledWidth { (texSize.h() * size().w())/size().h() };
            wallpaper.setSrcRect(LRectF((texSize.w() - outputScaledWidth)/2, 0.f, outputScaledWidth, texSize.h()));
        }
        else
            wallpaper.setSrcRect(LRectF(0.f, (texSize.h() - outputScaledHeight)/2, texSize.w(), outputScaledHeight));
    }
}

void Output::zoomedDrawBegin() noexcept
{
    /* Set the zone to capture */
    G::scene()->enableAutoRepaint(false);
    zoomScene.setParent(G::scene()->mainView());
    zoomScene.setVisible(true);
    zoomScene.setSizeB(sizeB() * zoom);
    zoomScene.setScale(scale());

    const LPointF outputRelativeCursorPos { cursor()->pos() - LPointF(pos()) };
    const LPointF outputNormalizedCursorPos { outputRelativeCursorPos / LSizeF(size()) };
    LPoint newPos { cursor()->pos() - (LSizeF(zoomScene.nativeSize()) * outputNormalizedCursorPos) };

    /* Prevent capturing stuff outside the output */

    if (newPos.x() < pos().x())
        newPos.setX(pos().x());
    else if (newPos.x() + zoomScene.nativeSize().w() > pos().x() + size().w())
        newPos.setX(pos().x() + size().w() - zoomScene.nativeSize().w());

    if (newPos.y() < pos().y())
        newPos.setY(pos().y());
    else if (newPos.y() + zoomScene.nativeSize().h() > pos().y() + size().h())
        newPos.setY(pos().y() + size().h() - zoomScene.nativeSize().h());

    zoomScene.setPos(newPos);

    /* Render views and the cursor manually into the zoom scene */
    G::compositor()->rootView.setParent(&zoomScene);
    zoomScene.render();

    /* Use an LTextureView to show the result scaled to the output size */
    zoomView.setDstSize(size());
    zoomView.setPos(pos());
    zoomView.setTexture(zoomScene.texture());
    zoomView.setParent(G::scene()->mainView());
    zoomScene.setVisible(false);
}

void Output::zoomedDrawEnd() noexcept
{
    /* Return everything to its place */
    G::compositor()->rootView.setParent(G::scene()->mainView());
    zoomView.setParent(nullptr);
    zoomScene.setParent(nullptr);
    G::scene()->enableAutoRepaint(true);
}

void Output::setZoom(Float32 newZoom) noexcept
{
    const Float32 prevZoom { zoom };
    zoom = newZoom;

    if (zoom > 1.f)
        zoom = 1.f;
    else if (newZoom < 0.25f)
        zoom = 0.25f;

    if (prevZoom != zoom)
        repaint();
}

void Output::setGammaRequest(LClient *client, const LGammaTable *gamma)
{
    L_UNUSED(client);
    setGamma(gamma);
}
