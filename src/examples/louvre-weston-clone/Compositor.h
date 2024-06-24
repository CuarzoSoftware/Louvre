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
    using LCompositor::LCompositor;

    void initialized() override;
    void uninitialized() override;

    LFactoryObject *createObjectRequest(LFactoryObject::Type type, const void *params) override;
    void onAnticipatedObjectDestruction(LFactoryObject *object) override;

    std::unique_ptr<LTexture> terminalIconTexture;
    std::unique_ptr<LXCursor> pointerCursor;
    std::unique_ptr<Clock> clock;
    std::list<DestroyedToplevel>destroyedToplevels;
};

#endif // COMPOSITOR_H
