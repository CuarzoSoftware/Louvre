#include <private/LSurfaceViewPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LSurfacePrivate.h>
#include <LSubsurfaceRole.h>
#include <LOutput.h>

using namespace Louvre;

LSurfaceView::LSurfaceView(LSurface *surface, LView *parent) :
    LView(LView::Surface, parent),
    LPRIVATE_INIT_UNIQUE(LSurfaceView)
{
    imp()->surface = surface;
    surface->imp()->views.push_back(this);
    enablePointerEvents(true);
    enableTouchEvents(true);
}

LSurfaceView::~LSurfaceView()
{
    if (imp()->customInputRegion)
        delete imp()->customInputRegion;

    if (imp()->customTranslucentRegion)
        delete imp()->customTranslucentRegion;

    if (surface())
    {
        LVectorRemoveOneUnordered(surface()->imp()->views, this);

        if (surface()->imp()->lastPointerEventView == this)
            surface()->imp()->lastPointerEventView = nullptr;
    }
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
    setCustomPos(pos.x(), pos.y());
}

void LSurfaceView::setCustomPos(Int32 x, Int32 y)
{
    if (customPosEnabled() && (x != imp()->customPos.x() || y != imp()->customPos.y()) && mapped())
        repaint();

    imp()->customPos.setX(x);
    imp()->customPos.setY(y);
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

void LSurfaceView::enableCustomTranslucentRegion(bool enabled)
{
    if (enabled != imp()->customTranslucentRegionEnabled)
    {
        imp()->customTranslucentRegionEnabled = enabled;
        repaint();
    }
}

bool LSurfaceView::customTranslucentRegionEnabled() const
{
    return imp()->customTranslucentRegionEnabled;
}

void LSurfaceView::setCustomTranslucentRegion(const LRegion *region)
{
    if (imp()->customTranslucentRegionEnabled)
        repaint();

    if (region)
    {
        if (imp()->customTranslucentRegion)
            *imp()->customTranslucentRegion = *region;
        else
        {
            imp()->customTranslucentRegion = new LRegion();
            *imp()->customTranslucentRegion = *region;
        }
    }
    else
    {
        if (imp()->customTranslucentRegion)
        {
            delete imp()->customTranslucentRegion;
            imp()->customTranslucentRegion = nullptr;
        }
    }
}

const LRectF &LSurfaceView::srcRect() const
{
    if (!imp()->surface)
        return imp()->customSrcRect;

    return imp()->surface->srcRect();
}

bool LSurfaceView::nativeMapped() const
{
    if (!imp()->surface)
        return false;

    return imp()->surface->mapped();
}

const LPoint &LSurfaceView::nativePos() const
{
    if (customPosEnabled() || !imp()->surface)
        return imp()->customPos;

    return imp()->surface->rolePos();
}

const LSize &LSurfaceView::nativeSize() const
{
    if (!imp()->surface)
        return imp()->customPos;

    return imp()->surface->size();
}

Float32 LSurfaceView::bufferScale() const
{
    if (!imp()->surface)
        return 1;

    return imp()->surface->bufferScale();
}

void LSurfaceView::enteredOutput(LOutput *output)
{
    if (imp()->primary && imp()->surface)
        imp()->surface->sendOutputEnterEvent(output);
    else
        LVectorPushBackIfNonexistent(imp()->nonPrimaryOutputs, output);
}

void LSurfaceView::leftOutput(LOutput *output)
{
    if (imp()->primary && imp()->surface)
        imp()->surface->sendOutputLeaveEvent(output);
    else
        LVectorRemoveOneUnordered(imp()->nonPrimaryOutputs, output);
}

const std::vector<LOutput *> &LSurfaceView::outputs() const
{
    if (imp()->primary && imp()->surface)
        return imp()->surface->outputs();
    else
        return imp()->nonPrimaryOutputs;
}

bool LSurfaceView::isRenderable() const
{
    return true;
}

void LSurfaceView::requestNextFrame(LOutput *output)
{
    if (!imp()->surface || !output)
        return;

    if (forceRequestNextFrameEnabled())
    {
        imp()->surface->requestNextFrame();
        m_threadsMap[output->threadId()].lastRenderedDamageId = imp()->surface->damageId();
        return;
    }

    if (!imp()->primary)
        return;

    bool clearDamage { true };

    for (LOutput *o : surface()->outputs())
    {
        // If the view is visible on another output and has not rendered the new damage
        // prevent clearing the damage immediately
        if (o != output && (m_threadsMap[o->threadId()].lastRenderedDamageId < imp()->surface->damageId()))
        {
            clearDamage = false;
            o->repaint();
        }
    }

    if (clearDamage)
    {
        imp()->surface->requestNextFrame();

        if (imp()->surface->subsurface() && imp()->surface->subsurface()->isSynced() && imp()->surface->parent())
            imp()->surface->parent()->requestNextFrame(false);
    }

    m_threadsMap[output->threadId()].lastRenderedDamageId = imp()->surface->damageId();
}

const LRegion *LSurfaceView::damage() const
{
    if (!imp()->surface)
        return nullptr;

    return &imp()->surface->damage();
}

const LRegion *LSurfaceView::translucentRegion() const
{
    if (imp()->customTranslucentRegionEnabled || !imp()->surface)
        return imp()->customTranslucentRegion;

    return &imp()->surface->translucentRegion();
}

const LRegion *LSurfaceView::opaqueRegion() const
{
    if (imp()->customTranslucentRegionEnabled || !imp()->surface)
        return nullptr;

    return &imp()->surface->opaqueRegion();
}

const LRegion *LSurfaceView::inputRegion() const
{
    if (customInputRegionEnabled() || !imp()->surface)
        return imp()->customInputRegion;

    return &imp()->surface->inputRegion();
}

void LSurfaceView::paintEvent(const PaintEventParams &params)
{
    if (!imp()->surface)
        return;

    params.painter->bindTextureMode({
        .texture = imp()->surface->texture(),
        .pos = pos(),
        .srcRect = srcRect(),
        .dstSize = size(),
        .srcTransform = imp()->surface->bufferTransform(),
        .srcScale = bufferScale(),
    });

    params.painter->enableCustomTextureColor(false);
    params.painter->drawRegion(*params.region);
}
