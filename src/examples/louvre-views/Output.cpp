#include <Output.h>
#include <Compositor.h>
#include <LCursor.h>
#include <LAnimation.h>
#include <LLayerView.h>
#include <LSurfaceView.h>
#include <LPainter.h>
#include <LLog.h>

Output::Output():LOutput()
{
    topBar.setVisible(false);
}

Compositor *Output::compositor() const
{
    return (Compositor*)LCompositor::compositor();
}

void Output::initializeGL()
{    
    /*
    compositor()->scene.mainView().enableClipping(false);
    compositor()->scene.mainView().setCustomPosC(posC());
    compositor()->scene.mainView().setCustomSize(sizeC());*/
    topBar.setPosC(rectC().pos());
    topBar.setScaledSizeC(LSize(sizeC().w(), 32*compositor()->globalScale()));
    topBar.setParent(&compositor()->overlayLayer);
    topBar.setVisible(true);
    compositor()->scene.handleInitializeGL(this);

    /*
    LAnimation *anim = new LAnimation(
        10000,
        [](LAnimation *anim)->bool
        {
            LScene *scene = (LScene*)anim->data();
            Int32 size = 100 + (1.f + sinf(anim->value()*20)) * 1000;
            //scene->mainView().setCustomSize(size);
            scene->mainView().setOpacity(sinf(anim->value()*30));
            return true;
        },
        [](LAnimation *anim)
        {
            LScene *scene = (LScene*)anim->data();
            scene->mainView().setCustomPosC(LPoint(0, 0));
            scene->mainView().setOpacity(1.f);
        },
        &compositor()->scene);

    anim->start();*/

}

void Output::resizeGL()
{
    /*compositor()->scene.mainView().setCustomPosC(posC());
    compositor()->scene.mainView().setCustomSize(sizeC());*/
    topBar.setPosC(rectC().pos());
    topBar.setScaledSizeC(LSize(sizeC().w(), 32*compositor()->globalScale()));
    compositor()->scene.handleResizeGL(this);
}

void Output::paintGL()
{
    //painter()->clearScreen();

    // Some times a surface may move under the cursor so we call this to update the cursor
    LSurfaceView *view = (LSurfaceView*)compositor()->scene.viewAtC(cursor()->posC());
    if (view && view->type() == LView::Surface && view->surface() != seat()->pointer()->focusSurface())
        seat()->pointer()->pointerPosChangeEvent(cursor()->posC().x(), cursor()->posC().y());

    compositor()->scene.handlePaintGL(this);
}

void Output::uninitializeGL()
{
    compositor()->scene.handleUninitializeGL(this);
}
