#include <private/LCompositorPrivate.h>
#include <LAnimation.h>
#include <algorithm>

using namespace Louvre;

LAnimation::LAnimation(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish) :
    m_onUpdate(onUpdate),
    m_duration(durationMs),
    m_onFinish(onFinish)
{
    compositor()->imp()->animations.push_back(this);
}

LAnimation::~LAnimation()
{
    stop();

    auto it { std::find(compositor()->imp()->animations.begin(), compositor()->imp()->animations.end(), this) };
    if (it != compositor()->imp()->animations.end())
    {
        compositor()->imp()->animationsVectorChanged = true;
        compositor()->imp()->animations.erase(it);
    }
}
