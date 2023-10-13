#include <LCursor.h>
#include <LAnimation.h>
#include <LLayerView.h>
#include <LSurfaceView.h>
#include <LPainter.h>
#include <LLog.h>
#include <LOpenGL.h>
#include <LTextureView.h>

#include "Global.h"
#include "Output.h"
#include "Compositor.h"
#include "Dock.h"
#include "Topbar.h"
#include "Workspace.h"
#include "Toplevel.h"
#include "Surface.h"
#include "ToplevelView.h"

Output::Output() : LOutput() {}

void Output::loadWallpaper()
{
    if (wallpaperView)
    {
        if (wallpaperView->texture())
        {
            if (sizeB() == wallpaperView->texture()->sizeB())
            {
                wallpaperView->setBufferScale(scale());
                return;
            }

            delete wallpaperView->texture();
            wallpaperView->setTexture(nullptr);
        }
    }
    else
    {
        wallpaperView = new LTextureView(nullptr, &G::compositor()->backgroundLayer);
        wallpaperView->enableParentOffset(false);
    }

    char wallpaperPath[256];
    sprintf(wallpaperPath, "%s/.config/Louvre/wallpaper.jpg", getenv("HOME"));
    LTexture *tmpWallpaper = LOpenGL::loadTexture(wallpaperPath);

    if (!tmpWallpaper)
        tmpWallpaper = LOpenGL::loadTexture("/usr/etc/Louvre/assets/wallpaper.jpg");

    if (tmpWallpaper)
    {
        // Clip and scale wallpaper so that it covers the entire screen

        LRect srcB;
        float w = float(size().w() * tmpWallpaper->sizeB().h()) / float(size().h());

        if (w >= tmpWallpaper->sizeB().w())
        {
            srcB.setX(0);
            srcB.setW(tmpWallpaper->sizeB().w());
            srcB.setH((tmpWallpaper->sizeB().w() * size().h()) / size().w());
            srcB.setY((tmpWallpaper->sizeB().h() - srcB.h()) / 2);
        }
        else
        {
            srcB.setY(0);
            srcB.setH(tmpWallpaper->sizeB().h());
            srcB.setW((tmpWallpaper->sizeB().h() * size().w()) / size().h());
            srcB.setX((tmpWallpaper->sizeB().w() - srcB.w()) / 2);
        }
        wallpaperView->setTexture(tmpWallpaper->copyB(sizeB(), srcB));
        wallpaperView->setBufferScale(scale());
        delete tmpWallpaper;
    }
    else
    {
        wallpaperView->setVisible(false);
    }

    LRegion trans;
    wallpaperView->setTranslucentRegion(&trans);
    wallpaperView->setPos(pos());
}

void Output::setWorkspace(Workspace *ws, UInt32 animMs, Float32 curve, Float32 start)
{
    animStart = start;
    easingCurve = curve;
    workspaceAnim->stop();
    workspaceAnim->setDuration(animMs);
    currentWorkspace = ws;

    for (Output *o : G::outputs())
        o->workspaces.front()->stealChildren();

    workspaceAnim->start(false);
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
                ws->toplevel->configure(size(), ws->toplevel->states());
        }

        offset += size().w() + spacing;
    }
}

void Output::initializeGL()
{
    workspaceAnim = LAnimation::create(400,
        [this](LAnimation *anim)
        {
            repaint();

            if (swippingWorkspace)
            {
                anim->stop();
                return;
            }

            // Hide non visible workspaces
            for (Workspace *ws : workspaces)
                ws->setVisible(LRect(ws->pos() + pos(), size()).intersects(rect()));

            Float32 ease = 1.f - powf(animStart + (1.f - animStart) * anim->value(), easingCurve);

            workspaceOffset = workspaceOffset * ease + Float32( - currentWorkspace->nativePos().x()) * (1.f - ease);
            workspacesContainer->setPos(workspaceOffset, 0);

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

                // Scaling vector for the black toplevel background container so it matches cSize
                LSizeF sVector;

                if (tl->fullscreen())
                {
                    // Fades in black background
                    tl->blackFullscreenBackground.setOpacity(anim->value() * anim->value());

                    // Fades out the unfullscreen toplevel capture
                    tl->capture.setOpacity(1.f - anim->value());
                    tl->capture.enableParentOpacity(false);
                    tl->animScene->render();
                    tl->animView.setTexture(tl->animScene->texture());
                    tl->animView.enableDstSize(true);
                    tl->animView.setPos((pos() * anim->value()) + (tl->prevBoundingRect.pos() * (1.f - anim->value())));
                    cSize = (size() * anim->value()) + (tl->prevBoundingRect.size() * (1.f - anim->value()));
                    tl->animView.setDstSize(cSize);
                }
                else
                {
                    tl->animScene->setPos(pos());
                    LPoint animPos = (pos() * (1.f - anim->value())) + (tl->prevBoundingRect.pos() * anim->value());
                    tl->surf()->setPos(0);
                    LBox box = tl->surf()->getView()->boundingBox();
                    tl->animScene->setSizeB(LSize(box.x2 - box.x1, box.y2 - box.y1)*2);
                    cSize = (size() * (1.f - anim->value())) + (LSize(box.x2 - box.x1, box.y2 - box.y1) * anim->value());

                    // Fades out black background
                    tl->blackFullscreenBackground.setOpacity(1.f - anim->value());

                    // Moves the black background from the output pos to the toplevel prev pos
                    tl->blackFullscreenBackground.setPos(animPos);
                    tl->blackFullscreenBackground.setSize(cSize);
                    tl->capture.setDstSize(cSize);

                    if (tl->decoratedView)
                        tl->surf()->setPos(LPoint() - (LPoint(box.x1, box.y1) - tl->animScene->nativePos()));
                    else
                        tl->surf()->setPos(tl->windowGeometry().pos());

                    tl->animScene->render();
                    tl->animView.setTexture(tl->animScene->texture());
                    tl->animView.enableDstSize(true);
                    tl->animView.enableParentOffset(false);
                    tl->animView.setPos(animPos);
                    tl->animView.setDstSize(cSize);
                    tl->animView.setOpacity(anim->value());
                    tl->configure(tl->prevRect.size(), LToplevelRole::Activated);
                }

                if (tl->decoratedView)
                    tl->decoratedView->updateGeometry();
            }
        },
        [this](LAnimation *)
        {
            if (currentWorkspace->toplevel)
            {
                Toplevel *tl = currentWorkspace->toplevel;

                tl->blackFullscreenBackground.setVisible(false);

                if (tl->capture.texture())
                {
                    delete tl->capture.texture();
                    tl->capture.setTexture(nullptr);
                }

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
                tl->configure(tl->states() | LToplevelRole::Activated);
            }

            if (animatedFullscreenToplevel)
            {
                Toplevel *tl = animatedFullscreenToplevel;

                tl->blackFullscreenBackground.setVisible(false);

                if (tl->capture.texture())
                {
                    delete tl->capture.texture();
                    tl->capture.setTexture(nullptr);
                }

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
                if (!o->swippingWorkspace)
                    o->currentWorkspace->returnChildren();

            updateWorkspacesPos();
            G::scene()->mainView()->damageAll(this);
            repaint();
        });

    workspacesContainer = new LLayerView(&G::compositor()->workspacesLayer);
    workspacesContainer->enableParentOffset(false);
    workspacesContainer->setPos(0, 0);
    currentWorkspace = new Workspace(this);

    new Topbar(this);
    topbar->update();

    new Dock(this);
    loadWallpaper();
    G::compositor()->scene.handleInitializeGL(this);
}

void Output::resizeGL()
{
    G::arrangeOutputs();
    updateWorkspacesPos();
    setWorkspace(currentWorkspace, 1);
    topbar->update();
    dock->update();
    loadWallpaper();
    G::compositor()->scene.handleResizeGL(this);
}

void Output::moveGL()
{
    updateWorkspacesPos();
    topbar->update();
    dock->update();
    wallpaperView->setPos(pos());
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
        seat()->pointer()->pointerMoveEvent(0, 0);
        G::compositor()->updatePointerBeforePaint = false;
    }

    // Check if hw cursor is supported
    if (cursor()->hasHardwareSupport(this))
        G::compositor()->softwareCursor.setTexture(nullptr);
    else
    {
        G::compositor()->softwareCursor.setTexture(cursor()->texture());
        G::compositor()->softwareCursor.setPos(cursor()->rect().pos());
        G::compositor()->softwareCursor.setDstSize(cursor()->rect().size());
    }

    G::compositor()->scene.handlePaintGL(this);
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

    // Unfullscreen toplevels
    while (workspaces.size() != 1)
    {
        Toplevel *tl = workspaces.back()->toplevel;
        tl->surf()->sendOutputEnterEvent(aliveOutput);
        tl->outputUnplugConfigureCount = 0;
        tl->prevStates = LToplevelRole::Activated;
        tl->prevRect.setPos(LPoint(0, TOPBAR_HEIGHT));
        tl->configure(tl->prevRect.size(), LToplevelRole::Activated);
        tl->quickUnfullscreen = true;
        tl->unsetFullscreen();
        tl->surf()->localOutputPos = tl->prevRect.pos() - pos();
        tl->surf()->localOutputSize = size();
        tl->surf()->outputUnplugHandled = false;
        workspaceAnim->stop();
    }

    workspacesContainer->setPos(0, 0);

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
            if (s->minimizeAnim)
                s->minimizeAnim->stop();

            s->minimizedOutput = aliveOutput;
            s->minimizeStartRect.setPos(LPoint(rand() % 128, TOPBAR_HEIGHT + (rand() % 128)));
        }
    }

    delete dock;
    dock = nullptr;
    delete topbar;
    topbar = nullptr;

    G::setViewTextureAndDestroyPrev(wallpaperView, nullptr);

    delete wallpaperView;
    wallpaperView = nullptr;

    workspaceAnim->stop();
    workspaceAnim->destroy();
    workspaceAnim = nullptr;

    while (!workspaces.empty())
        delete workspaces.back();

    delete workspacesContainer;
    workspacesContainer = nullptr;

    currentWorkspace = nullptr;
    animatedFullscreenToplevel = nullptr;

    G::compositor()->scene.handleUninitializeGL(this);
}
