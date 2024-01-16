#ifndef LANIMATIONPRIVATE_H
#define LANIMATIONPRIVATE_H

#include <LAnimation.h>

using namespace Louvre;

LPRIVATE_CLASS(LAnimation)
    Float32 value = 0.f;
    UInt32 duration;
    UInt32 beginTime;
    bool processed = false;
    bool pendingDestroy = false;
    bool running = false;
    bool destroyOnFinish = false;
    Callback onUpdate = nullptr;
    Callback onFinish = nullptr;
};

#endif // LANIMATIONPRIVATE_H
