#include <private/LSurfaceViewPrivate.h>
#include <private/LViewPrivate.h>
#include <LSurface.h>
#include <LOutput.h>
#include <LPainter.h>

using namespace Louvre;

LSurfaceView::LSurfaceView(LSurface *surface, bool primary, bool useCustomPos, LView *parent) : LView(LView::Surface, parent)
{
    m_imp = new LSurfaceViewPrivate();
    imp()->surface = surface;
    setPrimary(primary);
    enableCustomPos(useCustomPos);
    enableInput(true);
}

LSurfaceView::~LSurfaceView()
{
    delete m_imp;
}

LSurface *LSurfaceView::surface() const
{
    return imp()->surface;
}

bool LSurfaceView::primary() const
{
    return imp()->primary;
}

void LSurfaceView::setPrimary(bool primary) const
{
    imp()->primary = primary;
}

bool LSurfaceView::customPosEnabled() const
{
    return imp()->customPosEnabled;
}

void LSurfaceView::enableCustomPos(bool enable) const
{
    imp()->customPosEnabled = enable;
}

void LSurfaceView::setCustomPosC(const LPoint &customPosC)
{
    imp()->customPosC = customPosC;
}

const LPoint &LSurfaceView::customPosC()
{
    return imp()->customPosC;
}

const LRegion *LSurfaceView::inputRegionC() const
{
    return &surface()->inputRegionC();
}

void LSurfaceView::paintRect(LPainter *p, Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH, Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH, Float32 scale, Float32 alpha)
{
    p->drawTextureC(surface()->texture(),
                    srcX, srcY,
                    srcW, srcH,
                    dstX, dstY,
                    dstW, dstH,
                    scale,
                    alpha);
}

bool LSurfaceView::mapped() const
{
    if (parent())
        return parent()->mapped() && visible() && surface()->mapped() && !surface()->minimized();

    return visible() && surface()->mapped() && !surface()->minimized();
}

const LPoint &LSurfaceView::posC() const
{
    if (parent())
    {
        if (customPosEnabled())
            imp()->tmpPosC = imp()->customPosC + parent()->posC();
        else
            imp()->tmpPosC = surface()->rolePosC() + parent()->posC();

        return imp()->tmpPosC;
    }
    else
    {
        if (customPosEnabled())
            return imp()->customPosC;
        else
            return surface()->rolePosC();
    }
}

const LSize &LSurfaceView::sizeC() const
{
    return surface()->sizeC();
}

Int32 LSurfaceView::scale() const
{
    return surface()->bufferScale();
}

void LSurfaceView::enterOutput(LOutput *output)
{
    surface()->sendOutputEnterEvent(output);
}

void LSurfaceView::leaveOutput(LOutput *output)
{
    surface()->sendOutputLeaveEvent(output);
}

const std::list<LOutput *> &LSurfaceView::outputs() const
{
    return surface()->outputs();
}

bool LSurfaceView::isRenderable() const
{
    return true;
}

bool LSurfaceView::forceRequestNextFrameEnabled() const
{
    return imp()->forceRequestNextFrameEnabled;
}

void LSurfaceView::enableForceRequestNextFrame(bool enabled)
{
    imp()->forceRequestNextFrameEnabled = enabled;
}

void LSurfaceView::requestNextFrame(LOutput *output)
{
    LView *view = this;

    if (forceRequestNextFrameEnabled())
    {
        surface()->requestNextFrame();
        view->imp()->outputsMap[output].lastRenderedDamageId = surface()->damageId();
        return;
    }

    bool clearDamage = true;
    for (LOutput *o : surface()->outputs())
    {
        // If the view is visible on another output and has not rendered the new damage
        // prevent clearing the damage immediately
        if (o != output && (view->imp()->outputsMap[o].lastRenderedDamageId < surface()->damageId()))
        {
            clearDamage = false;
            o->repaint();
        }
    }
    surface()->requestNextFrame(clearDamage);
    view->imp()->outputsMap[output].lastRenderedDamageId = surface()->damageId();
}

const LRegion *LSurfaceView::damageC() const
{
    return &surface()->damagesC();
}

const LRegion *LSurfaceView::translucentRegionC() const
{
    return &surface()->translucentRegionC();
}

const LRegion *LSurfaceView::opaqueRegionC() const
{
    return &surface()->opaqueRegionC();
}
