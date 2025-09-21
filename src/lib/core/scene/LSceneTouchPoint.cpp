#include <LSceneTouchPoint.h>
#include <LTouchDownEvent.h>
#include <private/LScenePrivate.h>

#include <algorithm>

using namespace Louvre;

LSceneTouchPoint::LSceneTouchPoint(LScene *scene,
                                   const LTouchDownEvent &event) noexcept
    : m_scene(scene), m_pos(event.pos()), m_id(event.id()) {
  scene->imp()->touchPoints.push_back(this);
}

std::vector<LSceneTouchPoint *>::iterator LSceneTouchPoint::destroy() noexcept {
  std::vector<LSceneTouchPoint *>::iterator it =
      std::find(m_scene->imp()->touchPoints.begin(),
                m_scene->imp()->touchPoints.end(), this);
  it = m_scene->imp()->touchPoints.erase(it);
  m_scene->imp()->state.add(LScene::LScenePrivate::TouchPointsVectorChanged);
  delete this;
  return it;
}
