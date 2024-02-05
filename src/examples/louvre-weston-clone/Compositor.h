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

class Compositor : public LCompositor
{
public:
    Compositor();

    LSeat *createSeatRequest(const void *params) override;
    LOutput *createOutputRequest(const void *params) override;
    LSurface *createSurfaceRequest(const void *params) override;
    LToplevelRole *createToplevelRoleRequest(const void *params) override;
    LPopupRole *createPopupRoleRequest(const void *params) override;
    LPointer *createPointerRequest(const void *params) override;

    void destroySurfaceRequest(LSurface *surface) override;
    void cursorInitialized() override;

    LXCursor *pointerCursor = nullptr;
    Clock *clock = nullptr;

    std::list<DestroyedToplevel>destroyedToplevels;
};

#endif // COMPOSITOR_H
