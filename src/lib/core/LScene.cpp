#include <private/LScenePrivate.h>
#include <private/LViewPrivate.h>
#include <LOutput.h>
#include <LCursor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LDNDIconRole.h>
#include <LPointer.h>

LScene::LScene()
{
    m_imp = new LScenePrivate();
    LView *view = &imp()->mainView;
    view->imp()->scene = this;
}

LScene::~LScene()
{
    delete m_imp;
}

void LScene::handleInitializeGL(LOutput *output)
{
    LScenePrivate::OutputData oD;
    oD.c = compositor();
    oD.p = output->painter();
    oD.o = output;
    imp()->outputsMap[output] = oD;
}

void LScene::handlePaintGL(LOutput *output)
{
    LScenePrivate::OutputData *oD = &imp()->outputsMap[output];
    imp()->clearTmpVariables(oD);
    imp()->checkOutputsScale(oD);
    imp()->checkRectChange(oD);
    imp()->calcNewDamage(&mainView(), oD);

    // Save new damage for next frame and add old damage to current damage
    if (output->buffersCount() > 1)
    {
        LRegion oldDamage = oD->prevDamageC;
        oD->prevDamageC = oD->newDamageC;
        oD->newDamageC.addRegion(oldDamage);
    }

    glDisable(GL_BLEND);
    imp()->drawOpaqueDamage(&mainView(), oD);
    imp()->drawBackground(oD);

    glEnable(GL_BLEND);
    imp()->drawTranslucentDamage(&mainView(), oD);

    oD->o->setBufferDamageC(oD->newDamageC);
    oD->newDamageC.clear();
}

void LScene::handleResizeGL(LOutput *output)
{
    LScenePrivate::OutputData *oD = &imp()->outputsMap[output];
    imp()->damageAll(oD);
}

void LScene::handleUninitializeGL(LOutput *output)
{
    auto it = imp()->outputsMap.find(output);

    if (it != imp()->outputsMap.end())
        imp()->outputsMap.erase(it);
}

LLayerView &LScene::mainView() const
{
    return imp()->mainView;
}

const LRGBF &LScene::clearColor() const
{
    return imp()->clearColor;
}

void LScene::setClearColor(const LRGBF &color)
{
    imp()->clearColor = color;
}

void LScene::setClearColor(Float32 r, Float32 g, Float32 b)
{
    imp()->clearColor = {r, g, b};
}

LView *LScene::viewAtC(const LPoint &pos)
{
    return imp()->viewAtC(&mainView(), pos);
}
