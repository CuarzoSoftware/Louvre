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
    workspaceAnim->start(false);
}

void Output::updateWorkspacesPos()
{
    Int32 offset = 0;
    Int32 spacing = 64;

    for (Workspace *ws : workspaces)
    {
        ws->setPos(offset, 0);
        offset += size().w() + spacing;
    }
}

void Output::initializeGL()
{
    workspaceAnim = LAnimation::create(400,
        [this](LAnimation *anim)
        {
            for (Workspace *ws : workspaces)
                ws->setVisible(LRect(ws->pos(), size()).intersects(rect()));

            if (swippingWorkspace)
            {
                anim->stop();
                return;
            }

            Float32 ease = 1.f - powf(animStart + (1.f - animStart) * anim->value(), easingCurve);

            workspaceOffset = workspaceOffset * ease + Float32( - currentWorkspace->nativePos().x()) * (1.f - ease);

            if (abs(workspaceOffset - currentWorkspace->nativePos().x()) < 2)
            {
                anim->stop();
                workspaceOffset = currentWorkspace->nativePos().x();
            }

            workspacesContainer->setPos(workspaceOffset, 0);

            for (Workspace *ws : workspaces)
                ws->clipChildren();

            if (currentWorkspace->toplevel && currentWorkspace->toplevel->animatingFullscreen)
            {
                Toplevel *tl = currentWorkspace->toplevel;
                tl->blackFullscreenBackground.setOpacity(anim->value());
                tl->blackFullscreenBackground.setPos((pos() * anim->value()) + (tl->prevBoundingRect.pos() * (1.f - anim->value())));

                Float32 exp = anim->value();

                LSize cSize = (size() * exp) + (tl->prevBoundingRect.size() * (1.f - exp));

                LSizeF sVector;
                sVector.setW(Float32(cSize.w()) / Float32(size().w()));
                sVector.setH(Float32(cSize.h()) / Float32(size().h()));

                tl->blackFullscreenBackground.setScalingVector(sVector);
                tl->capture.setOpacity(1.f - exp);

                Surface *surf = (Surface*)tl->surface();
                surf->getView()->enableParentScaling(true);
                surf->getView()->setOpacity(1.f);
                surf->getView()->setVisible(true);
                surf->getView()->setParent(&tl->blackFullscreenBackground);
                surf->setPos(0,0);
                surf->raise();

                if (tl->decoratedView)
                    tl->decoratedView->updateGeometry();
            }

            repaint();
        },
        [this](LAnimation *anim)
        {
            if (currentWorkspace->toplevel)
            {
                Toplevel *tl = currentWorkspace->toplevel;

                if (tl->animatingFullscreen)
                {
                    tl->blackFullscreenBackground.setParent(&currentWorkspace->surfaces);
                    tl->blackFullscreenBackground.enableParentOffset(true);
                    tl->blackFullscreenBackground.setPos(0,0);
                    tl->animatingFullscreen = false;
                    delete tl->capture.texture();
                    tl->capture.setTexture(nullptr);
                }

                seat()->pointer()->setFocus(tl->surface());
                seat()->keyboard()->setFocus(tl->surface());
                tl->configure(tl->states() | LToplevelRole::Activated);
            }
            else if(currentWorkspace == workspaces.front())
            {
                currentWorkspace->returnChildren();
            }
        });

    workspacesContainer = new LLayerView(&G::compositor()->workspacesLayer);
    currentWorkspace = new Workspace(this);

    new Topbar(this);
    topbar->update();

    new Dock(this);
    loadWallpaper();
    G::compositor()->scene.handleInitializeGL(this);
}

void Output::resizeGL()
{
    updateWorkspacesPos();
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
    G::compositor()->scene.handleMoveGL(this);
}

void Output::paintGL()
{
    //painter()->clearScreen();

    if (G::compositor()->updatePointerBeforePaint)
    {
        seat()->pointer()->pointerMoveEvent(0, 0);
        G::compositor()->updatePointerBeforePaint = false;
    }

    G::compositor()->scene.handlePaintGL(this);
}

void Output::uninitializeGL()
{
    delete dock;
    delete topbar;
    G::compositor()->scene.handleUninitializeGL(this);
}
