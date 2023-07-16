#include <private/LSurfaceViewPrivate.h>
#include <private/LViewPrivate.h>
#include <LSurface.h>
#include <LOutput.h>
#include <LPainter.h>

using namespace Louvre;

LSurfaceView::LSurfaceView(LSurface *surface, LView *parent) : LView(LView::Surface, parent)
{
    m_imp = new LSurfaceViewPrivate();
    imp()->surface = surface;
    enableInput(true);
}

LSurfaceView::~LSurfaceView()
{
    if (imp()->customInputRegion)
        delete imp()->customInputRegion;

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

void LSurfaceView::setPrimary(bool primary)
{
    imp()->primary = primary;
}

bool LSurfaceView::customPosEnabled() const
{
    return imp()->customPosEnabled;
}

void LSurfaceView::enableCustomPos(bool enable)
{
    imp()->customPosEnabled = enable;
}

bool LSurfaceView::customInputRegionEnabled() const
{
    return imp()->customInputRegionEnabled;
}

void LSurfaceView::enableCustomInputRegion(bool enabled)
{
    if (enabled != imp()->customInputRegionEnabled && mapped())
        repaint();

    imp()->customInputRegionEnabled = enabled;
}

void LSurfaceView::setCustomPosC(const LPoint &pos)
{
    if (customPosEnabled() && pos != imp()->customPos && mapped())
        repaint();

    imp()->customPos = pos;
}

const LPoint &LSurfaceView::customPosC() const
{
    return imp()->customPos;
}

void LSurfaceView::setCustomInputRegionC(const LRegion *region)
{
    if (region)
    {
        if (imp()->customInputRegion)
            *imp()->customInputRegion = *region;
        else
        {
            imp()->customInputRegion = new LRegion();
            *imp()->customInputRegion = *region;
        }
    }
    else
    {
        if (imp()->customInputRegion)
        {
            delete imp()->customInputRegion;
            imp()->customInputRegion = nullptr;
        }
    }
}

const LRegion *LSurfaceView::customInputRegionC() const
{
    return imp()->customInputRegion;
}

bool LSurfaceView::nativeMapped() const
{
    return surface()->mapped();
}

const LPoint &LSurfaceView::nativePosC() const
{
    if (customPosEnabled())
        return imp()->customPos;

    return surface()->rolePosC();
}

const LSize &LSurfaceView::nativeSizeC() const
{
    return surface()->sizeC();
}

Int32 LSurfaceView::bufferScale() const
{
    return surface()->bufferScale();
}

void LSurfaceView::enteredOutput(LOutput *output)
{
    if (primary())
        surface()->sendOutputEnterEvent(output);
}

void LSurfaceView::leftOutput(LOutput *output)
{
    if (primary())
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

    if (clearDamage)
        surface()->requestNextFrame();

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

const LRegion *LSurfaceView::inputRegionC() const
{
    if (customInputRegionEnabled())
        return imp()->customInputRegion;

    return &surface()->inputRegionC();
}

void LSurfaceView::paintRectC(LPainter *p,
                              Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                              Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                              Float32 scale, Float32 alpha)
{
    p->drawTextureC(surface()->texture(),
                    srcX, srcY,
                    srcW, srcH,
                    dstX, dstY,
                    dstW, dstH,
                    scale,
                    alpha);
}
