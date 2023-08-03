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

void Output::updateTopBar()
{
    topBarView->setPos(pos());
    topBarView->setSize(LSize(size().w(), 24));
}

void Output::initializeGL()
{    
    topBarView = new LSolidColorView(1.f, 1.f, 1.f, 0.6f, G::compositor()->overlayLayer);
    topBarView->enableParentOffset(false);
    updateTopBar();
    dock = new Dock(this);
    loadWallpaper();
    G::compositor()->scene->handleInitializeGL(this);
}

void Output::resizeGL()
{
    updateTopBar();
    dock->update();
    loadWallpaper();
    G::compositor()->scene->handleResizeGL(this);
}

void Output::moveGL()
{
    updateTopBar();
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
    dock = nullptr;
    delete topBarView;
    topBarView = nullptr;
    G::compositor()->scene->handleUninitializeGL(this);
}
