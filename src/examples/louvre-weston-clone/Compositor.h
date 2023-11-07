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

    LSeat *createSeatRequest(LSeat::Params *params) override;
    LOutput *createOutputRequest() override;
    LSurface *createSurfaceRequest(LSurface::Params *params) override;
    LToplevelRole *createToplevelRoleRequest(LToplevelRole::Params *params) override;
    LPopupRole *createPopupRoleRequest(LPopupRole::Params *params) override;
    LPointer *createPointerRequest(LPointer::Params *params) override;


    void destroySurfaceRequest(LSurface *surface) override;

    void cursorInitialized() override;

    LXCursor *pointerCursor = nullptr;
    Clock *clock = nullptr;

    std::list<DestroyedToplevel>destroyedToplevels;
};

#endif // COMPOSITOR_H
