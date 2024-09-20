#include <private/LCompositorPrivate.h>
#include <LAnimation.h>
#include <algorithm>
#include <LTime.h>

using namespace Louvre;

LAnimation::LAnimation(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish) noexcept :
    m_onUpdate(onUpdate),
    m_duration(durationMs),
    m_onFinish(onFinish)
{
    compositor()->imp()->animations.push_back(this);
}

LAnimation::~LAnimation()
{
    notifyDestruction();
    stop();

    auto it { std::find(compositor()->imp()->animations.begin(), compositor()->imp()->animations.end(), this) };
    if (it != compositor()->imp()->animations.end())
    {
        compositor()->imp()->animationsVectorChanged = true;
        compositor()->imp()->animations.erase(it);
    }
}

void LAnimation::oneShot(UInt32 durationMs, const Callback &onUpdate, const Callback &onFinish) noexcept
{
    LAnimation *anim { new LAnimation(durationMs, onUpdate, onFinish) };
    anim->m_destroyOnFinish = true;
    anim->start();
}

void LAnimation::setOnUpdateCallback(const Callback &onUpdate) noexcept
{
    if (m_running)
        return;

    m_onUpdate = onUpdate;
}

void LAnimation::setOnFinishCallback(const Callback &onFinish) noexcept
{
    if (m_running)
        return;

    m_onFinish = onFinish;
}

void LAnimation::start() noexcept
{
    if (m_running)
        return;

    m_value = 0.0;
    m_beginTime = std::chrono::steady_clock::now();
    m_running = true;
    compositor()->repaintAllOutputs();
}

void LAnimation::stop()
{
    if (!m_running)
        return;

    m_value = 1.0;
    m_running = false;

    if (m_onFinish)
        m_onFinish(this);

    if (m_destroyOnFinish)
        m_pendingDestroy = true;
}
