#ifndef LANIMATION_H
#define LANIMATION_H

#include <LObject.h>
#include <functional>

class Louvre::LAnimation : public LObject
{
public:
    using Callback = std::function<void(LAnimation*)>;
    ~LAnimation();

    static void oneShot(UInt32 durationMs, Callback onUpdate = nullptr, Callback onFinish = nullptr);
    static LAnimation *create(UInt32 durationMs, Callback onUpdate = nullptr, Callback onFinish = nullptr);

    void setOnUpdateCallback(Callback onUpdate);
    void setOnFinishCallback(Callback onFinish);
    void setDuration(UInt32 durationMs);

    UInt32 duration() const;
    Float32 value() const;

    void start(bool destroyOnFinish = true);
    void stop();

LPRIVATE_IMP(LAnimation)
    LAnimation();
};

#endif // LANIMATION_H
