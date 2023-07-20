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

void LSurfaceView::setCustomPos(const LPoint &pos)
{
    if (customPosEnabled() && pos != imp()->customPos && mapped())
        repaint();

    imp()->customPos = pos;
}

const LPoint &LSurfaceView::customPos() const
{
    return imp()->customPos;
}

void LSurfaceView::setCustomInputRegion(const LRegion *region)
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

const LRegion *LSurfaceView::customInputRegion() const
{
    return imp()->customInputRegion;
}

bool LSurfaceView::nativeMapped() const
{
    return surface()->mapped();
}

const LPoint &LSurfaceView::nativePos() const
{
    if (customPosEnabled())
        return imp()->customPos;

    return surface()->rolePos();
}

const LSize &LSurfaceView::nativeSize() const
{
    return surface()->size();
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

const LRegion *LSurfaceView::damage() const
{
    return &surface()->damage();
}

const LRegion *LSurfaceView::translucentRegion() const
{
    return &surface()->translucentRegion();
}

const LRegion *LSurfaceView::opaqueRegion() const
{
    return &surface()->opaqueRegion();
}

const LRegion *LSurfaceView::inputRegion() const
{
    if (customInputRegionEnabled())
        return imp()->customInputRegion;

    return &surface()->inputRegion();
}

void LSurfaceView::paintRect(LPainter *p,
                              Int32 srcX, Int32 srcY, Int32 srcW, Int32 srcH,
                              Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                              Float32 scale, Float32 alpha)
{
    p->drawTexture(surface()->texture(),
                    srcX, srcY,
                    srcW, srcH,
                    dstX, dstY,
                    dstW, dstH,
                    scale,
                    alpha);
}
