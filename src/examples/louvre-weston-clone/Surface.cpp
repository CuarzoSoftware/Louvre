#include "Surface.h"
#include "LCursor.h"
#include "Output.h"
#include "Compositor.h"

Surface::Surface(LSurface::Params *params, GLuint textureUnit) : LSurface(params, textureUnit)
{

}

void Surface::repaint()
{
    for (LOutput *o : outputs())
    {
        if (outputParams.find(o) != outputParams.end())
        {
            outputParams[o].changedOrder[0] = true;
            outputParams[o].changedOrder[1] = true;
        }
    }
}

void Surface::mappingChanged()
{
    if (mapped())
    {
        if (firstMap)
        {
            firstMap = false;

            Int32 barSize = 32 * compositor()->globalScale();
            LPoint outputPosG = compositor()->cursor()->output()->posC() + LPoint(0, barSize);
            LSize outputSizeG = compositor()->cursor()->output()->sizeC() - LSize(0,barSize);

            setPosC(outputPosG + outputSizeG/2 - sizeC()/2);

            if (posC().x() < outputPosG.x())
                setXC(outputPosG.x());
            if (posC().y() < barSize)
                setYC(barSize);
        }
        repaint();
        compositor()->repaintAllOutputs();
    }
    else
    {
        for (Output *o : (list<Output*>&)outputs())
        {
            o->newDamage.addRect(LRect(rolePosC(),sizeC()));
            o->repaint();
        }
    }
}

void Surface::orderChanged()
{
    changedOrder = true;
    repaint();
}

UInt64 Surface::allOutputsRequestedNewFrame()
{
    UInt64 total = 0;

    for (LOutput *o : outputs())
    {
        if (outputParams[o].requestedNewFrame)
            total++;
    }

    return total;
}
