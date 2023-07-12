#include <LCompositor.h>
#include "Compositor.h"
#include "LTime.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"

Surface::Surface(LSurface::Params *params, GLuint textureUnit) : LSurface(params, textureUnit)
{
   view = new LSurfaceView(this, compositor()->surfacesLayer);
}

Surface::~Surface()
{
    delete view;
}

Compositor *Surface::compositor() const
{
    return (Compositor*)LCompositor::compositor();
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
                Int32 barSize = 0 * compositor()->globalScale();
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
        view->repaint();
    }
}

void Surface::orderChanged()
{
    Surface *prev = (Surface*)prevSurface();

    if (prev)
        view->insertAfter(prev->view, false);
    else
        view->insertAfter(nullptr, false);
}

void Surface::roleChanged()
{
    if (roleId() == LSurface::Cursor)
    {
        view->enableForceRequestNextFrame(true);
        view->setVisible(false);
        view->setParent(compositor()->hiddenCursorsLayer);
    }
}

void Surface::bufferSizeChanged()
{
    view->repaint();
}

void Surface::minimizedChanged()
{
    if (minimized())
    {
        if (toplevel())
            toplevel()->configureC(0);
    }
    else
    {
        compositor()->raiseSurface(this);
        if (toplevel())
            toplevel()->configureC(LToplevelRole::Activated);
    }
}
