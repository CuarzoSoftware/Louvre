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

    LSeat *createSeatRequest(void *params) override;
    LOutput *createOutputRequest() override;
    LSurface *createSurfaceRequest(void *params) override;
    LToplevelRole *createToplevelRoleRequest(void *params) override;
    LPopupRole *createPopupRoleRequest(void *params) override;
    LPointer *createPointerRequest(void *params) override;

    void destroySurfaceRequest(LSurface *surface) override;
    void cursorInitialized() override;

    LXCursor *pointerCursor = nullptr;
    Clock *clock = nullptr;

    std::list<DestroyedToplevel>destroyedToplevels;
};

#endif // COMPOSITOR_H
