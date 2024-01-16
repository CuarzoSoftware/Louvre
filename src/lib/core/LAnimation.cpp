#include <private/LAnimationPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LTime.h>
#include <algorithm>

LAnimation::LAnimation(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish) : LPRIVATE_INIT_UNIQUE(LAnimation)
{
    compositor()->imp()->animations.push_back(this);
    imp()->duration = durationMs;
    imp()->onUpdate = onUpdate;
    imp()->onFinish = onFinish;
}

LAnimation::~LAnimation()
{
    stop();

    auto it = std::find(compositor()->imp()->animations.begin(), compositor()->imp()->animations.end(), this);
    if (it != compositor()->imp()->animations.end())
    {
        compositor()->imp()->animationsVectorChanged= true;
        compositor()->imp()->animations.erase(it);
    }
}

void LAnimation::oneShot(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish)
{
    LAnimation *anim = new LAnimation(durationMs, onUpdate, onFinish);
    anim->imp()->destroyOnFinish = true;
    anim->start();
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

void LAnimation::start()
{
    if (imp()->running)
        return;

    imp()->value = 0.f;
    imp()->beginTime = LTime::ms();
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

bool LAnimation::running() const
{
    return imp()->running;
}
