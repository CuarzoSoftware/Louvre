#include <LCompositor.h>
#include "Compositor.h"
#include "LTime.h"
#include "Surface.h"
#include "LCursor.h"
#include "Output.h"
#include <Shared.h>
#include <LTextureView.h>
#include <Dock.h>
#include <LAnimation.h>

Surface::Surface(LSurface::Params *params) : LSurface(params)
{
    view = new LSurfaceView(this, compositor()->surfacesLayer);
}

Surface::~Surface()
{
    if (minimizeAnim)
        minimizeAnim->stop();

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
        if (!toplevel())
        {
            view->setVisible(false);
            return;
        }

        view->enableInput(false);
        minimizedTexture = texture()->copyB(LSize((minimizedItemHeight()*texture()->sizeB().w())/texture()->sizeB().h(), minimizedItemHeight()));

        for (Output *o : outps())
        {
            LTextureView *minView = new LTextureView(minimizedTexture, &o->dock->background);
            minView->setBufferScale(comp()->globalScale());
            minView->enableScaling(true);
            minView->enableParentOpacity(false);
            minimizedViews.push_back(minView);

            posBeforeMinimized = posC();
            view->enableScaling(true);

            minimizeAnim = LAnimation::create(300,
            [this, minView, o](LAnimation *anim)
            {
                Float32 expVal = 1.f - powf(1.f - anim->value(), 2.f);
                minView->setScalingVector(expVal);
                view->setScalingVector(1.f - expVal);
                setPosC((minView->posC() + minView->sizeC()) * expVal +
                         posBeforeMinimized * (1.f - expVal));
                o->dock->update();
                return true;
            },
            [this, minView, o](LAnimation *anim)
            {
                minView->setScalingVector(anim->value());
                minView->enableScaling(false);
                o->dock->update();
                view->setVisible(false);
                view->enableScaling(false);
                minimizeAnim = nullptr;
            });

            minimizeAnim->start();
        }

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
