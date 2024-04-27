#include <LCompositor.h>
#include <LToplevelRole.h>
#include "Compositor.h"
#include "LTime.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"

Surface::Surface(const void *params) noexcept : LSurface(params) {}

void Surface::mappingChanged()
{
    if (mapped())
    {
        if (firstMap)
        {
            firstMap = false;

            if (toplevel() && cursor()->output())
            {
                const Int32 barSize { 32 };
                const LPoint outputPosG { cursor()->output()->pos() + LPoint(0, barSize) };
                const LSize outputSizeG { cursor()->output()->size() - LSize(0, barSize) };

                setPos(outputPosG + (outputSizeG - toplevel()->windowGeometry().size())/2);

                if (pos().x() < outputPosG.x())
                    setX(outputPosG.x());

                if (pos().y() < barSize)
                    setY(barSize);
            }
        }

        compositor()->repaintAllOutputs();
    }
    else
    {
        if (toplevel())
        {
            Compositor *c { (Compositor*)compositor() };
            DestroyedToplevel destroyed;
            destroyed.texture = texture()->copy();
            destroyed.rect = LRect(rolePos(), size());
            destroyed.ms = LTime::ms();

            for (LOutput *o : outputs())
            {
                if (o->rect().intersects(destroyed.rect))
                {
                    destroyed.outputs.push_back(o);
                    o->repaint();
                }
            }
            c->destroyedToplevels.push_back(destroyed);
        }
        else
        {
            for (Output *o : (std::vector<Output*>&)outputs())
            {
                o->newDamage.addRect(outputsMap[o].previousRect);
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
        for (Output *o : (std::vector<Output*>&)outputs())
        {
            o->newDamage.addRect(outputsMap[o].previousRect);
            o->repaint();

            if (this == o->fullscreenSurface)
                o->fullscreenSurface = nullptr;
        }

        if (toplevel())
            toplevel()->configureState(toplevel()->pending().state &~ LToplevelRole::Activated);
    }
    else
    {
        requestNextFrame(false);
        raise();

        if (toplevel())
            toplevel()->configureState(LToplevelRole::Activated);
    }
}
