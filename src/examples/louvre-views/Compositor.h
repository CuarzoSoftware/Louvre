#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <LCompositor.h>
#include <LScene.h>
#include <LView.h>

using namespace Louvre;

class Compositor : public LCompositor
{
public:
    Compositor();

    LOutput *createOutputRequest() override;
    LSurface *createSurfaceRequest(LSurface::Params *params) override;
    LPointer *createPointerRequest(LPointer::Params *params) override;

    LScene scene;
};

#endif // COMPOSITOR_H
