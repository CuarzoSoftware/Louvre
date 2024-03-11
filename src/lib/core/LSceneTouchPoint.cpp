#include <private/LSceneTouchPointPrivate.h>
#include <private/LScenePrivate.h>
#include <LTouchDownEvent.h>
#include <algorithm>

using namespace Louvre;

LSceneTouchPoint::LSceneTouchPoint(LScene *scene, const LTouchDownEvent &event) :
    LPRIVATE_INIT_UNIQUE(LSceneTouchPoint)
{
    imp()->scene = scene;
    imp()->id = event.id();
    imp()->pos = event.pos();
    scene->imp()->touchPoints.push_back(this);
}

LSceneTouchPoint::~LSceneTouchPoint() {}

std::vector<LSceneTouchPoint*>::iterator LSceneTouchPoint::destroy()
{
    std::vector<LSceneTouchPoint*>::iterator it = std::find(imp()->scene->imp()->touchPoints.begin(), imp()->scene->imp()->touchPoints.end(), this);
    it = imp()->scene->imp()->touchPoints.erase(it);
    imp()->scene->imp()->state.add(LScene::LScenePrivate::TouchPointsVectorChanged);
    delete this;
    return it;
}

LScene *LSceneTouchPoint::scene() const
{
    return imp()->scene;
}

Int32 LSceneTouchPoint::id() const
{
    return imp()->id;
}

bool LSceneTouchPoint::isPressed() const
{
    return imp()->isPressed;
}

const std::vector<LView *> &LSceneTouchPoint::views() const
{
    return imp()->views;
}

const LPointF &LSceneTouchPoint::pos() const
{
    return imp()->pos;
}

