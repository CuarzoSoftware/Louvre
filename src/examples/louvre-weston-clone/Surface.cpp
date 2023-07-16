#include <LCompositor.h>
#include "Compositor.h"
#include "LTime.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"

Surface::Surface(LSurface::Params *params) : LSurface(params)
{

}

void Surface::mappingChanged()
{
    if (mapped())
    {
        if (firstMap)
        {
            firstMap = false;

            if (toplevel())
            {
                Int32 barSize = 32 * compositor()->globalScale();
                LPoint outputPosG = compositor()->cursor()->output()->posC() + LPoint(0, barSize);
                LSize outputSizeG = compositor()->cursor()->output()->sizeC() - LSize(0, barSize);

                setPosC(outputPosG + outputSizeG/2 - toplevel()->windowGeometryC().size()/2);

                if (posC().x() < outputPosG.x())
                    setXC(outputPosG.x());

                if (posC().y() < barSize)
                    setYC(barSize);

                toplevel()->configureC(LToplevelRole::Activated);
            }
        }

        compositor()->repaintAllOutputs();
    }
    else
    {
        if (toplevel())
        {
            Compositor *c = (Compositor*)compositor();
            DestroyedToplevel destroyed;
            destroyed.texture = texture()->copyB();
            destroyed.rect = LRect(rolePosC(), sizeC());
            destroyed.ms = LTime::ms();

            for (LOutput *o : outputs())
            {
                if (o->rectC().intersects(destroyed.rect))
                {
                    destroyed.outputs.push_back(o);
                    o->repaint();
                }
            }
            c->destroyedToplevels.push_back(destroyed);
        }
        else
        {
            for (Output *o : (list<Output*>&)outputs())
            {
                o->newDamage.addRect(outputsMap[o].previousRectC);
                o->repaint();
                if (this == o->fullscreenSurface)
                    o->fullscreenSurface = nullptr;
            }
        }
    }
}

void Surface::orderChanged()
{
    for (auto &pair : outputsMap)
        pair.second.changedOrder = true;

    repaintOutputs();
}

void Surface::minimizedChanged()
{
    if (minimized())
    {
        for (Output *o : (list<Output*>&)outputs())
        {
            o->newDamage.addRect(outputsMap[o].previousRectC);
            o->repaint();

            if (this == o->fullscreenSurface)
                o->fullscreenSurface = nullptr;
        }

        if (toplevel())
            toplevel()->configureC(toplevel()->states() &~ LToplevelRole::Activated);
    }
    else
    {
        requestNextFrame(false);
        compositor()->raiseSurface(this);

        if (toplevel())
            toplevel()->configureC(LToplevelRole::Activated);
    }
}
