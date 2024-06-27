#include <LCompositor.h>
#include <LToplevelRole.h>
#include "Compositor.h"
#include "LTime.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"

void Surface::mappingChanged()
{
    if (mapped())
    {
        if (firstMap)
        {
            firstMap = false;

            if (toplevel() && cursor()->output())
            {
                const LRect availGeo {
                    cursor()->output()->pos() + cursor()->output()->availableGeometry().pos(),
                    cursor()->output()->availableGeometry().size() };

                setPos(availGeo.pos()
                       + (availGeo.size() - toplevel()->windowGeometry().size())/2);

                if (pos().x() < availGeo.x())
                    setX(availGeo.x());

                if (pos().y() < availGeo.y())
                    setY(availGeo.y());
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
            toplevel()->configureState(toplevel()->pendingConfiguration().state &~ LToplevelRole::Activated);
    }
    else
    {
        requestNextFrame(false);
        raise();

        if (toplevel())
            toplevel()->configureState(LToplevelRole::Activated);
    }
}
