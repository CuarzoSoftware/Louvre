#include <private/LAnimationPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LTime.h>

LAnimation::LAnimation(UInt32 durationMs,
                       bool (*onUpdate)(LAnimation *animation),
                       void (*onFinish)(LAnimation *animation),
                       void *data)
{
    m_imp = new LAnimationPrivate();
    setDuration(durationMs);
    this->onUpdate(onUpdate);
    this->onFinish(onFinish);
    setData(data);
    LCompositor::compositor()->imp()->animations.push_back(this);
    imp()->compositorLink = std::prev(LCompositor::compositor()->imp()->animations.end());
}

LAnimation::~LAnimation()
{
    delete m_imp;
}

void LAnimation::onUpdate(bool (*onUpdate)(LAnimation *animation))
{
    imp()->onUpdate = onUpdate;
}

void LAnimation::onFinish(void (*onFinish)(LAnimation *animation))
{
    imp()->onFinish = onFinish;
}

void LAnimation::setDuration(UInt32 durationMs)
{
    imp()->duration = durationMs;
}

UInt32 LAnimation::duration() const
{
    return imp()->duration;
}

void LAnimation::setData(void *data)
{
    imp()->data = data;
}

void *LAnimation::data()
{
    return imp()->data;
}

Float32 LAnimation::value() const
{
    return imp()->value;
}

void LAnimation::start(bool destroyOnFinish)
{
    imp()->value = 0.f;
    imp()->beginTime = LTime::ms();
    imp()->deleteOnFinish = destroyOnFinish;
    imp()->running = true;
}
