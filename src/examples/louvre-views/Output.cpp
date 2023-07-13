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
#include <Shared.h>

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
        wallpaperView = new LTextureView(nullptr, comp()->backgroundLayer);
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

    wallpaperView->setNativePosC(posC());
}

void Output::initializeGL()
{    
    dock = new Dock(this);
    loadWallpaper();
    compositor()->scene->handleInitializeGL(this);
}

void Output::resizeGL()
{
    loadWallpaper();
    compositor()->scene->handleResizeGL(this);
}

void Output::paintGL()
{
    //painter()->clearScreen();

    // Some times a surface may move under the cursor so we call this to update the cursor
    LSurfaceView *view = (LSurfaceView*)compositor()->scene->viewAtC(cursor()->posC());
    if (view && view->type() == LView::Surface && view->surface() != seat()->pointer()->focusSurface())
        seat()->pointer()->pointerPosChangeEvent(cursor()->posC().x(), cursor()->posC().y());

    compositor()->scene->handlePaintGL(this);
}

void Output::uninitializeGL()
{
    compositor()->scene->handleUninitializeGL(this);
    delete dock;
    dock = nullptr;
}
