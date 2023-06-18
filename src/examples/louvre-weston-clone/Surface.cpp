#include <LCompositor.h>
#include "Compositor.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"

Surface::Surface(LSurface::Params *params, GLuint textureUnit) : LSurface(params, textureUnit)
{

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
        compositor()->repaintAllOutputs();
    }
    else
    {
        for (Output *o : (list<Output*>&)outputs())
        {
            o->newDamage.addRect(LRect(rolePosC(), sizeC()));
            o->repaint();
        }
    }
}

void Surface::orderChanged()
{
    for (auto &pair : outputsMap)
        pair.second.changedOrder = true;

    repaintOutputs();
}
