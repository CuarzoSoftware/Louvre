#include <Compositor.h>
#include <Output.h>

Compositor::Compositor():LCompositor(){}


LOutput *Compositor::createOutputRequest()
{
    return new Output();
}




