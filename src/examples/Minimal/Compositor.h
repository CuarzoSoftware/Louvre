#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <LCompositor.h>

using namespace Louvre;

class Compositor : public LCompositor
{
public:
    Compositor();
    LOutput *createOutputRequest() override;
    LSurface *createSurfaceRequest(LSurface::Params *params) override;

};

#endif // COMPOSITOR_H
