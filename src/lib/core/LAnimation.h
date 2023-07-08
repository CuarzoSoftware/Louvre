#ifndef LANIMATION_H
#define LANIMATION_H

#include <LObject.h>

class Louvre::LAnimation : public LObject
{
public:
    LAnimation(UInt32 durationMs,
               bool (*onUpdate)(LAnimation *animation) = nullptr,
               void (*onFinish)(LAnimation *animation) = nullptr,
               void *data = nullptr);
    ~LAnimation();
    void onUpdate(bool (*onUpdate)(LAnimation *animation));
    void onFinish(void (*onFinish)(LAnimation *animation));
    void setDuration(UInt32 durationMs);
    UInt32 duration() const;
    void setData(void *data);
    void *data();
    Float32 value() const;
    void start(bool destroyOnFinish = true);
LPRIVATE_IMP(LAnimation)
};

#endif // LANIMATION_H
