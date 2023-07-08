#ifndef LANIMATIONPRIVATE_H
#define LANIMATIONPRIVATE_H

#include <LAnimation.h>

using namespace Louvre;

LPRIVATE_CLASS(LAnimation)
    Float32 value = 0.f;
    UInt32 duration;
    UInt32 beginTime;
    bool running = false;
    bool deleteOnFinish = true;
    void *data = nullptr;
    std::list<LAnimation*>::iterator compositorLink;
    bool (*onUpdate)(LAnimation *animation) = nullptr;
    void (*onFinish)(LAnimation *animation) = nullptr;
};

#endif // LANIMATIONPRIVATE_H
