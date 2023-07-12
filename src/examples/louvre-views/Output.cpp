#include <Output.h>
#include <Compositor.h>
#include <LCursor.h>
#include <LAnimation.h>
#include <LLayerView.h>
#include <LSurfaceView.h>
#include <LPainter.h>
#include <LLog.h>

Output::Output():LOutput() {}

Compositor *Output::compositor() const
{
    return (Compositor*)LCompositor::compositor();
}

void Output::initializeGL()
{    
    topBar = new LSolidColorView(1.f, 1.f, 1.f, 0.5);
    topBar->setNativePosC(rectC().pos());
    topBar->setNativeSizeC(LSize(sizeC().w(), 32*compositor()->globalScale()));
    topBar->setParent(compositor()->overlayLayer);
    topBar->setVisible(true);
    topBar->enableParentClipping(false);
    compositor()->scene->handleInitializeGL(this);

    /*
    LAnimation *anim = new LAnimation(
        10000,
        [](LAnimation *anim)->bool
        {
            LLayerView *layer= (LLayerView*)anim->data();
            //Int32 size = 100 + (1.f + sinf(anim->value()*20)) * 1000;
            //layer->setNativeSizeC(size);
            layer->setScalingVector(LPointF(1.f - anim->value()));
            layer->parent()->setOpacity(anim->value()+0.5);
            return true;
        },
        [](LAnimation *anim)
        {
            LLayerView *layer= (LLayerView*)anim->data();
            layer->setScalingVector(0.75f);
        },
        compositor()->surfacesLayer);

    anim->start();
    */
}

void Output::resizeGL()
{
    topBar->setNativePosC(rectC().pos());
    topBar->setNativeSizeC(LSize(sizeC().w(), 32*compositor()->globalScale()));
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
    delete topBar;
}
