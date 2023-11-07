#include <private/LAnimationPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LTime.h>

void LAnimation::oneShot(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish)
{
    LAnimation *anim = new LAnimation();
    anim->imp()->duration = durationMs;
    anim->imp()->onUpdate = onUpdate;
    anim->imp()->onFinish = onFinish;
    anim->start();
}

LAnimation *LAnimation::create(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish)
{
    LAnimation *anim = new LAnimation();
    anim->imp()->duration = durationMs;
    anim->imp()->onUpdate = onUpdate;
    anim->imp()->onFinish = onFinish;
    return anim;
}

void LAnimation::setOnUpdateCallback(const Callback &onUpdate)
{
    if (imp()->running)
        return;

    imp()->onUpdate = onUpdate;
}

void LAnimation::setOnFinishCallback(const Callback &onFinish)
{
    if (imp()->running)
        return;

    imp()->onFinish = onFinish;
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
    if (imp()->running)
        return;

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

    imp()->running = false;

    if (imp()->onFinish)
        imp()->onFinish(this);

    if (imp()->destroyOnFinish)
        imp()->pendingDestroy = true;
}

void LAnimation::destroy()
{
    imp()->pendingDestroy = true;
}

LAnimation::LAnimation()
{
    m_imp = new LAnimationPrivate();
    LCompositor::compositor()->imp()->animations.push_back(this);
    imp()->compositorLink = std::prev(LCompositor::compositor()->imp()->animations.end());
}

LAnimation::~LAnimation()
{
    delete m_imp;
}
