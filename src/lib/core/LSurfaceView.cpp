#include <private/LPainterPrivate.h>
#include <private/LSurfacePrivate.h>
#include <LSubsurfaceRole.h>
#include <LOutput.h>

using namespace Louvre;

LSurfaceView::LSurfaceView(LSurface *surface, LView *parent) noexcept : LView(LView::Surface, true, parent), m_surface(surface)
{
    setPrimary(true);
    surface->imp()->views.push_back(this);
    enablePointerEvents(true);
    enableTouchEvents(true);
}

LSurfaceView::~LSurfaceView() noexcept
{
    if (surface())
        LVectorRemoveOneUnordered(surface()->imp()->views, this);
}

const LRectF &LSurfaceView::srcRect() const
{
    if (surface())
        return surface()->srcRect();

    return m_tmpSrcRect;
}

bool LSurfaceView::nativeMapped() const noexcept
{
    if (surface())
        return surface()->mapped();

    return false;
}

const LPoint &LSurfaceView::nativePos() const noexcept
{
    if (customPosEnabled() || !surface())
        return m_customPos;

    return surface()->rolePos();
}

const LSize &LSurfaceView::nativeSize() const noexcept
{
    if (surface())
        return surface()->size();

    return m_tmpSize;
}

Float32 LSurfaceView::bufferScale() const noexcept
{
    if (surface())
        return surface()->bufferScale();

    return 1;
}

void LSurfaceView::enteredOutput(LOutput *output) noexcept
{
    if (primary() && surface())
        surface()->sendOutputEnterEvent(output);

    LVectorPushBackIfNonexistent(m_nonPrimaryOutputs, output);
}

void LSurfaceView::leftOutput(LOutput *output) noexcept
{
    if (primary() && surface())
        surface()->sendOutputLeaveEvent(output);

    LVectorRemoveOneUnordered(m_nonPrimaryOutputs, output);
}

const std::vector<LOutput *> &LSurfaceView::outputs() const noexcept
{
    if (primary() && surface())
        return surface()->outputs();
    else
        return m_nonPrimaryOutputs;
}

void LSurfaceView::requestNextFrame(LOutput *output) noexcept
{
    if (!surface() || !output)
        return;

    if (forceRequestNextFrameEnabled())
    {
        surface()->requestNextFrame();
        m_threadsMap[output->threadId()].lastRenderedDamageId = surface()->damageId();
        return;
    }

    if (!primary())
        return;

    bool clearDamage { true };

    for (LOutput *o : surface()->outputs())
    {
        // If the view is visible on another output and has not rendered the new damage
        // prevent clearing the damage immediately
        if (o != output && (m_threadsMap[o->threadId()].lastRenderedDamageId < surface()->damageId()))
        {
            clearDamage = false;
            o->repaint();
        }
    }

    if (clearDamage)
    {
        surface()->requestNextFrame();

        if (surface()->subsurface() && surface()->subsurface()->isSynced() && surface()->parent())
            surface()->parent()->requestNextFrame(false);
    }

    m_threadsMap[output->threadId()].lastRenderedDamageId = surface()->damageId();
}

const LRegion *LSurfaceView::damage() const noexcept
{
    if (surface())
        return &surface()->damage();

    return nullptr;
}

const LRegion *LSurfaceView::translucentRegion() const noexcept
{
    if (customTranslucentRegionEnabled() || !surface())
        return m_customTranslucentRegion.get();

    return &surface()->translucentRegion();
}

const LRegion *LSurfaceView::opaqueRegion() const noexcept
{
    if (customTranslucentRegionEnabled() || !surface())
        return nullptr;

    return &surface()->opaqueRegion();
}

const LRegion *LSurfaceView::inputRegion() const noexcept
{
    if (customInputRegionEnabled() || !surface())
        return m_customInputRegion.get();

    return &surface()->inputRegion();
}

void LSurfaceView::paintEvent(const PaintEventParams &params) noexcept
{
    if (!surface())
        return;

    params.painter->bindTextureMode({
        .texture = surface()->texture(),
        .pos = pos(),
        .srcRect = srcRect(),
        .dstSize = size(),
        .srcTransform = surface()->bufferTransform(),
        .srcScale = bufferScale(),
    });

    params.painter->enableCustomTextureColor(false);
    params.painter->drawRegion(*params.region);
}
