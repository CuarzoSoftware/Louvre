#include <Output.h>
#include <Compositor.h>
#include <LCursor.h>
#include <LAnimation.h>
#include <LLayerView.h>
#include <LSurfaceView.h>
#include <LLog.h>

Output::Output():LOutput(){}

Compositor *Output::compositor() const
{
    return (Compositor*)LCompositor::compositor();
}

void Output::initializeGL()
{
    compositor()->scene.handleInitializeGL(this);

    LAnimation *anim = new LAnimation(
        5000,
        [](LAnimation *anim)->bool
        {
            LScene *scene = (LScene*)anim->data();
            scene->mainView().setCustomPosC(LPoint(
                                                (Int32)(100*(1.f + sinf(anim->value()*10))),
                                                (Int32)(100*(1.f + cosf(anim->value()*10)))
                                            ));
            LLog::debug("Anim %f", anim->value());
            LCompositor::compositor()->repaintAllOutputs();
            return true;
        },
        [](LAnimation *anim)
        {
            LScene *scene = (LScene*)anim->data();
            scene->mainView().setCustomPosC(LPoint(200, 200));
        },
        &compositor()->scene);

    anim->start();

}

void Output::resizeGL()
{
    compositor()->scene.handleResizeGL(this);
}

void Output::paintGL()
{
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
