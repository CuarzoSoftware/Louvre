#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <private/LSceneViewPrivate.h>
#include <LOutput.h>
#include <LCursor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LPointer.h>

LScene::LScene()
{
    m_imp = new LScenePrivate();
    imp()->view = new LSceneView();
    imp()->view->enableCustomPos(true);

    LView *baseView = imp()->view;
    baseView->imp()->scene = this;
}

LScene::~LScene()
{
    delete imp()->view;
    delete m_imp;
}

void LScene::handleInitializeGL(LOutput *output)
{
    /*
    LSceneView::LSceneViewPrivate::OutputData oD;
    oD.c = compositor();
    oD.p = output->painter();
    oD.o = output;
    imp()->view->imp()->outputsMap[output] = oD;*/
}

void LScene::handlePaintGL(LOutput *output)
{
    imp()->view->imp()->fb = output->framebuffer();
    imp()->view->render(output);
}

void LScene::handleResizeGL(LOutput *output)
{
    /*
    LSceneView::LSceneViewPrivate::OutputData *oD = &imp()->view->imp()->outputsMap[output];
    imp()->view->imp()->damageAll(oD);*/
}

void LScene::handleUninitializeGL(LOutput *output)
{
    auto it = imp()->view->imp()->outputsMap.find(output);

    if (it != imp()->view->imp()->outputsMap.end())
        imp()->view->imp()->outputsMap.erase(it);
}

LSceneView *LScene::mainView() const
{
    return imp()->view;
}

LView *LScene::viewAtC(const LPoint &pos)
{
    return imp()->viewAtC(mainView(), pos);
}
