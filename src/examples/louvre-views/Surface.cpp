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
        minimizedTexture = texture()->copyB(LSize((minimizedItemHeight()*texture()->sizeB().w())/texture()->sizeB().h(), minimizedItemHeight()));

        for (Output *o : outps())
        {
            LTextureView *minView = new LTextureView(minimizedTexture, &o->dock->background);
            minView->setBufferScale(comp()->globalScale());
            minView->enableScaling(true);
            minView->enableParentOpacity(false);
            minimizedViews.push_back(minView);

            posBeforeMinimized = rolePosC();
            view->enableScaling(true);

            struct AnimData
            {
                Surface *surface;
                LTextureView *minView;
                Dock *dock;
            };

            AnimData *animData = new AnimData();
            animData->minView = minView;
            animData->surface = this;
            animData->dock = o->dock;

            (new LAnimation(200,
            [](LAnimation *anim)->bool
            {
                AnimData *animData = (AnimData*)anim->data();
                animData->minView->setScalingVector(anim->value());
                animData->surface->view->setScalingVector(1.f - anim->value());
                animData->surface->setPosC((animData->minView->posC() + animData->minView->sizeC())*anim->value() +
                                           animData->surface->posBeforeMinimized * (1.f - anim->value()));
                animData->dock->update();
                return true;
            },
            [](LAnimation *anim)
            {
                AnimData *animData = (AnimData*)anim->data();
                animData->minView->setScalingVector(anim->value());
                animData->minView->enableScaling(false);
                animData->dock->update();
                animData->surface->view->setVisible(false);
                animData->surface->view->enableScaling(false);
                delete animData;
            },
            animData))->start();
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
