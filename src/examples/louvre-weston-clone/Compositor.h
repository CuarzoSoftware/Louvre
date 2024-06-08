#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <LCompositor.h>
#include <LXCursor.h>
#include "Clock.h"

using namespace Louvre;

struct DestroyedToplevel
{
    LTexture *texture;
    Int32 ms;
    LRect rect;
    std::list<LOutput*>outputs;
};

class Compositor final : public LCompositor
{
public:
    Compositor() noexcept;

    void initialized() override;
    LFactoryObject *createObjectRequest(LFactoryObject::Type type, const void *params) override;
    void onAnticipatedObjectDestruction(LFactoryObject *object) override;

    LXCursor *pointerCursor { nullptr };
    Clock *clock { nullptr };

    std::list<DestroyedToplevel>destroyedToplevels;
};

#endif // COMPOSITOR_H
