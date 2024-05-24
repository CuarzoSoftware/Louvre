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
#include "Topbar.h"
#include "Workspace.h"
#include "Toplevel.h"
#include "Surface.h"
#include "ToplevelView.h"

Output::Output(const void *params) noexcept : LOutput(params)
{
    workspacesContainer.enableParentOffset(false);
    workspacesContainer.setPos(0, 0);

    wallpaper.enableDstSize(true);
    wallpaper.enableSrcRect(true);
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
    G::arrangeOutputs();
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

    if (needsFullRepaint())
        G::scene()->mainView()->damageAll(this);

    // Check pointer events before painting
    if (G::compositor()->updatePointerBeforePaint)
    {
        seat()->pointer()->pointerMoveEvent(LPointerMoveEvent());
        G::compositor()->updatePointerBeforePaint = false;
    }

    updateFractionalOversampling();
    G::scene()->handlePaintGL(this);

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
    G::compositor()->scene.handleUninitializeGL(this);
}

void Output::setWorkspace(Workspace *ws, UInt32 animMs, Float64 curve, Float64 start)
{
    animStartOffset = start;
    animEasingCurve = curve;
    workspacesAnimation.stop();
    workspacesAnimation.setDuration(animMs * DEBUG_ANIM_SPEED);
    currentWorkspace = ws;

    if (currentWorkspace->toplevel && currentWorkspace->toplevel->surf())
        enableVSync(currentWorkspace->toplevel->surf()->preferVSync());
    else
        enableVSync(true);

    topbar.update();

    for (Output *o : G::outputs())
        o->workspaces.front()->stealChildren();

    workspacesAnimation.start();
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
                ws->toplevel->configureState(ws->toplevel->pending().state);
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
            if (surf->toplevel() && surf->toplevel()->current().decorationMode == LToplevelRole::ServerSide && surf->mapped() && !surf->minimized())
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
        G::scene()->mainView()->damageAll(this);
        topbar.update();
    }
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

    const Float64 ease { 1.0 - pow(animStartOffset + (1.0 - animStartOffset) * anim->value(), animEasingCurve) };
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

        tl->surface()->requestNextFrame(false);

        // Current fullscreen size
        LSize cSize;

        if (tl->fullscreen())
        {
            Float32 val = 1.f - pow(1.0 - anim->value(), 4.0);
            Float32 inv = 1.f - val;
            tl->animView.enableSrcRect(false);
            tl->animView.setVisible(true);
            tl->animScene->render();
            tl->animView.setTexture(tl->animScene->texture());
            tl->animView.setPos((pos() * val) + (tl->prevBoundingRect.pos() * (inv)));
            cSize = (tl->fullscreenOutput->size() * val) + (tl->prevBoundingRect.size() * (inv));
            tl->animView.setDstSize(cSize);
            tl->animView.setOpacity(val);

            tl->capture.setPos(tl->animView.pos());
            tl->capture.setDstSize(cSize);
            LRegion transRegion = tl->captureTransRegion;
            LSizeF transRegionScale = LSizeF(cSize) / LSizeF(tl->prevBoundingRect.size());
            transRegion.multiply(transRegionScale.x(), transRegionScale.y());
            tl->capture.setTranslucentRegion(&transRegion);
        }
        else
        {
            Float32 val = 1.f - pow(1.f - anim->value(), 2.f);
            Float32 inv = 1.f - val;
            tl->animScene->setPos(pos());
            LPoint animPos = (pos() * inv) + (tl->prevBoundingRect.pos() * val);
            tl->surf()->setPos(0);
            LBox box = tl->surf()->getView()->boundingBox();
            LSize boxSize = LSize(box.x2 - box.x1, box.y2 - box.y1);
            cSize = (size() * inv) + (boxSize * val);

            tl->capture.setOpacity(inv);
            tl->capture.setPos(animPos);
            tl->capture.setDstSize(cSize);

            if (tl->decoratedView)
                tl->surf()->setPos(LPoint() - (LPoint(box.x1, box.y1) - tl->animScene->nativePos()));
            else
                tl->surf()->setPos(tl->windowGeometry().pos());

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
}

void Output::onWorkspacesAnimationFinish(LAnimation */*anim*/) noexcept
{
    hideAllWorkspacesExceptCurrent();

    if (currentWorkspace->toplevel)
    {
        Toplevel *tl = currentWorkspace->toplevel;

        tl->blackFullscreenBackground.setVisible(false);

        if (tl->capture.texture())
            delete tl->capture.texture();

        tl->animView.setTexture(nullptr);

        if (tl->animScene)
        {
            delete tl->animScene;
            tl->animScene = nullptr;
        }

        if (tl->destructorCalled || tl->quickUnfullscreen)
            goto returnChildren;

        seat()->pointer()->setFocus(tl->surface());
        seat()->keyboard()->setFocus(tl->surface());
        tl->configureState(tl->pending().state | LToplevelRole::Activated);
    }

    if (animatedFullscreenToplevel)
    {
        Toplevel *tl = animatedFullscreenToplevel;

        tl->blackFullscreenBackground.setVisible(false);

        if (tl->capture.texture())
            delete tl->capture.texture();

        tl->animView.setTexture(nullptr);

        if (tl->animScene)
        {
            delete tl->animScene;
            tl->animScene = nullptr;
        }

        if (tl->destructorCalled || tl->quickUnfullscreen)
            goto returnChildren;

        if (tl->fullscreen())
        {
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
    for (Output *o : G::outputs())
        if (!o->doingFingerWorkspaceSwipe)
            o->currentWorkspace->returnChildren();

    updateWorkspacesPos();
    G::scene()->mainView()->damageAll(this);
    repaint();
}

void Output::updateWallpaper() noexcept
{
    if (!G::textures()->wallpaper || size().area() == 0)
        return;

    wallpaper.setTexture(G::textures()->wallpaper);
    wallpaper.setPos(pos());
    wallpaper.setDstSize(size());

    const LSize &texSize { G::textures()->wallpaper->sizeB() };

    const Int32 outputScaledHeight { (texSize.w() * size().h())/size().w() };

    if (outputScaledHeight >= G::textures()->wallpaper->sizeB().h())
    {
        const Int32 outputScaledWidth { (texSize.h() * size().w())/size().h() };
        wallpaper.setSrcRect(LRectF((texSize.w() - outputScaledWidth)/2, 0.f, outputScaledWidth, texSize.h()));
    }
    else
    {
        wallpaper.setSrcRect(LRectF(0.f, (texSize.h() - outputScaledHeight)/2, texSize.w(), outputScaledHeight));
    }
}

void Output::setGammaRequest(LClient *client, const LGammaTable *gamma)
{
    L_UNUSED(client);
    setGamma(gamma);
}
