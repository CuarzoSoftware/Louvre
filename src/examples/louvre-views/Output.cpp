#include <Output.h>
#include <Compositor.h>
#include <Dock.h>
#include <LCursor.h>
#include <LAnimation.h>
#include <LLayerView.h>
#include <LSurfaceView.h>
#include <LPainter.h>
#include <LLog.h>
#include <LOpenGL.h>
#include <LTextureView.h>
#include <Global.h>

Output::Output():LOutput() {}

Compositor *Output::compositor() const
{
    return (Compositor*)LCompositor::compositor();
}

void Output::loadWallpaper()
{
    if (wallpaperView)
    {
        if (wallpaperView->texture())
        {
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

    LRegion trans;
    wallpaperView->setTranslucentRegion(&trans);
}

void Output::initializeGL()
{    
    dock = new Dock(this);
    loadWallpaper();
    compositor()->scene->handleInitializeGL(this);
}

void Output::resizeGL()
{
    dock->update();
    loadWallpaper();
    compositor()->scene->handleResizeGL(this);
}

void Output::moveGL()
{
    dock->update();
    wallpaperView->setPos(pos());
}

void Output::paintGL()
{
    compositor()->scene->handlePaintGL(this);
}

void Output::uninitializeGL()
{
    compositor()->scene->handleUninitializeGL(this);
    delete dock;
    dock = nullptr;
}
