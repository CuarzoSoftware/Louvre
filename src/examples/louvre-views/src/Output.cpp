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

Output::Output():LOutput() {}

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
        wallpaperView = new LTextureView(nullptr, G::compositor()->backgroundLayer);
        wallpaperView->enableParentOffset(false);
    }

    char wallpaperPath[256];
    sprintf(wallpaperPath, "%s/.config/louvre-weston-clone/wallpaper.jpg", getenv("HOME"));
    LTexture *tmpWallpaper = LOpenGL::loadTexture(wallpaperPath);

    if (tmpWallpaper)
    {
        wallpaperView->setTexture(tmpWallpaper->copyB(sizeB()));
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

void Output::initializeGL()
{
    new Topbar(this);
    topbar->update();

    fullscreenView = new LSolidColorView(0.f, 0.f, 0.f, 1.f, G::compositor()->fullscreenLayer);
    fullscreenView->setVisible(false);
    fullscreenView->enableInput(true);
    fullscreenView->enableBlockPointer(true);
    fullscreenView->setPos(pos());
    fullscreenView->setSize(size());

    new Dock(this);
    loadWallpaper();
    G::compositor()->scene->handleInitializeGL(this);
}

void Output::resizeGL()
{
    fullscreenView->setSize(size());
    topbar->update();
    dock->update();
    loadWallpaper();
    G::compositor()->scene->handleResizeGL(this);
}

void Output::moveGL()
{
    fullscreenView->setPos(pos());
    topbar->update();
    dock->update();
    wallpaperView->setPos(pos());
    G::compositor()->scene->handleMoveGL(this);
}

void Output::paintGL()
{
    //painter()->clearScreen();

    if (G::compositor()->updatePointerBeforePaint)
    {
        seat()->pointer()->pointerMoveEvent(0, 0);
        G::compositor()->updatePointerBeforePaint = false;
    }

    G::compositor()->scene->handlePaintGL(this);
}

void Output::uninitializeGL()
{
    delete dock;
    delete topbar;
    G::compositor()->scene->handleUninitializeGL(this);
}
