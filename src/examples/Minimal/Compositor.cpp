#include "Compositor.h"
#include "Output.h"
#include "Surface.h"

#include <LOutputMode.h>

Compositor::Compositor() : LCompositor()
{

}


LOutput *Compositor::createOutputRequest()
{
    return new Output();
}

LSurface *Compositor::createSurfaceRequest(LSurface::Params *params)
{
    return new Surface(params);
}
