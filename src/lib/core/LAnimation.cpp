#include <private/LAnimationPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LTime.h>

LAnimation::~LAnimation()
{
    delete m_imp;
}

void LAnimation::oneShot(UInt32 durationMs, Callback onUpdate, Callback onFinish)
{
    LAnimation *anim = new LAnimation();
    anim->imp()->duration = durationMs;
    anim->imp()->onUpdate = onUpdate;
    anim->imp()->onFinish = onFinish;
    anim->start();
}

LAnimation *LAnimation::create(UInt32 durationMs, Callback onUpdate, Callback onFinish)
{
    LAnimation *anim = new LAnimation();
    anim->imp()->duration = durationMs;
    anim->imp()->onUpdate = onUpdate;
    anim->imp()->onFinish = onFinish;
    return anim;
}

void LAnimation::setOnUpdateCallback(Callback onUpdate)
{
    if (imp()->running)
        return;

    imp()->onUpdate = onUpdate;
}

void LAnimation::setOnFinishCallback(Callback onFinish)
{
    if (imp()->running)
        return;

    imp()->onFinish= onFinish;
}

void LAnimation::setDuration(UInt32 durationMs)
{
    if (imp()->running)
        return;

    imp()->duration = durationMs;
}

UInt32 LAnimation::duration() const
{
    return imp()->duration;
}

Float32 LAnimation::value() const
{
    return imp()->value;
}

void LAnimation::start(bool destroyOnFinish)
{
    imp()->value = 0.f;
    imp()->beginTime = LTime::ms();
    imp()->destroyOnFinish = destroyOnFinish;
    imp()->running = true;
    compositor()->repaintAllOutputs();
}

void LAnimation::stop()
{
    if (!imp()->running)
        return;

    imp()->value = 1.f;
}

LAnimation::LAnimation()
{
    m_imp = new LAnimationPrivate();
    LCompositor::compositor()->imp()->animations.push_back(this);
    imp()->compositorLink = std::prev(LCompositor::compositor()->imp()->animations.end());
}
