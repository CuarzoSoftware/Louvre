#include <private/LCompositorPrivate.h>
#include <private/LViewPrivate.h>
#include <private/LScenePrivate.h>
#include <private/LSceneViewPrivate.h>
#include <private/LSceneTouchPointPrivate.h>
#include <LTouchCancelEvent.h>
#include <LOutput.h>
#include <LLog.h>
#include <string.h>

using namespace Louvre;

using LVS = LView::LViewPrivate::LViewState;

LView::LView(UInt32 type, LView *parent) : LPRIVATE_INIT_UNIQUE(LView)
{
    imp()->type = type;
    imp()->view = this;
    compositor()->imp()->views.push_back(this);
    setParent(parent);
}

LView::~LView()
{
    setParent(nullptr);

    while (!children().empty())
        children().front()->setParent(nullptr);

    LVectorRemoveOneUnordered(compositor()->imp()->views, this);
}

void LView::enableKeyboardEvents(bool enabled)
{
    if (enabled == keyboardEventsEnabled())
        return;

    imp()->setFlag(LVS::KeyboardEvents, enabled);

    if (scene())
    {
        if (enabled)
            scene()->imp()->keyboardFocus.push_back(this);
        else
            LVectorRemoveOne(scene()->imp()->keyboardFocus, this);

        scene()->imp()->state.add(LScene::LScenePrivate::KeyboardFocusVectorChanged);
    }
}

bool LView::keyboardEventsEnabled() const
{
    return imp()->hasFlag(LVS::KeyboardEvents);
}

void LView::enableTouchEvents(bool enabled)
{
    if (enabled == touchEventsEnabled())
        return;

    imp()->setFlag(LVS::TouchEvents, enabled);

    if (scene())
    {
        if (!enabled)
        {
            for (auto *tp : scene()->touchPoints())
            {
                for (auto it = tp->imp()->views.begin(); it != tp->views().end();)
                {
                    if ((*it) == this)
                    {
                        LView *v = *it;
                        it = tp->imp()->views.erase(it);
                        tp->imp()->listChanged = true;
                        v->touchCancelEvent(LTouchCancelEvent());
                    }
                    else
                        it++;
                }
            }
        }
    }
}

bool LView::touchEventsEnabled() const
{
    return imp()->hasFlag(LVS::TouchEvents);
}

LSceneTouchPoint *LView::findTouchPoint(Int32 id) const
{
    if (scene())
        for (auto *tp : scene()->touchPoints())
            if (tp->id() == id)
                return tp;

    return nullptr;
}

bool LView::pointerEventsEnabled() const
{
    return imp()->hasFlag(LVS::PointerEvents);
}

void LView::enablePointerEvents(bool enabled)
{
    if (enabled == pointerEventsEnabled())
        return;

    imp()->setFlag(LVS::PointerEvents, enabled);

    if (!enabled)
    {
        if (imp()->hasFlag(LVS::PointerIsOver))
        {
            if (scene())
            {
                if (imp()->hasFlag(LVS::PendingSwipeEnd))
                {
                    imp()->removeFlag(LVS::PendingSwipeEnd);
                    scene()->imp()->pointerSwipeEndEvent.setCancelled(true);
                    scene()->imp()->pointerSwipeEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                    scene()->imp()->pointerSwipeEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                    scene()->imp()->pointerSwipeEndEvent.setSerial(LTime::nextSerial());
                    pointerSwipeEndEvent(scene()->imp()->pointerSwipeEndEvent);
                }

                if (imp()->hasFlag(LVS::PendingPinchEnd))
                {
                    imp()->removeFlag(LVS::PendingPinchEnd);
                    scene()->imp()->pointerPinchEndEvent.setCancelled(true);
                    scene()->imp()->pointerPinchEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                    scene()->imp()->pointerPinchEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                    scene()->imp()->pointerPinchEndEvent.setSerial(LTime::nextSerial());
                    pointerPinchEndEvent(scene()->imp()->pointerPinchEndEvent);
                }

                if (imp()->hasFlag(LVS::PendingHoldEnd))
                {
                    imp()->removeFlag(LVS::PendingHoldEnd);
                    scene()->imp()->pointerHoldEndEvent.setCancelled(true);
                    scene()->imp()->pointerHoldEndEvent.setMs(scene()->imp()->currentPointerMoveEvent.ms());
                    scene()->imp()->pointerHoldEndEvent.setUs(scene()->imp()->currentPointerMoveEvent.us());
                    scene()->imp()->pointerHoldEndEvent.setSerial(LTime::nextSerial());
                    pointerHoldEndEvent(scene()->imp()->pointerHoldEndEvent);
                }

                LVectorRemoveOne(scene()->imp()->pointerFocus, this);
                scene()->imp()->state.add(LScene::LScenePrivate::PointerFocusVectorChanged);
            }

            imp()->removeFlag(LVS::PointerIsOver);
        }
    }
}

void LView::damageAll()
{
    imp()->markAsChangedOrder(false);

    if (mapped())
        repaint();
}

LScene *LView::scene() const
{
    // Only the LScene mainView has this variable assigned
    if (imp()->scene)
        return imp()->scene;

    if (parent())
        return parent()->scene();

    return nullptr;
}

LSceneView *LView::parentSceneView() const
{
    if (parent())
    {
        if (parent()->type() == Scene)
            return (LSceneView*)parent();

        return parent()->parentSceneView();
    }
    return nullptr;
}

UInt32 LView::type() const
{
    return imp()->type;
}

void LView::repaint()
{
    if (imp()->hasFlag(LVS::RepaintCalled))
        return;

    for (LOutput *o : outputs())
        o->repaint();

    imp()->addFlag(LVS::RepaintCalled);
}

LView *LView::parent() const
{
    return imp()->parent;
}

void LView::setParent(LView *view)
{
    if (parent() == view || view == this)
        return;

    LScene *s { scene() };

    if (s)
        s->imp()->state.add(LScene::LScenePrivate::ChildrenListChanged);

    if (parent())
        parent()->imp()->children.erase(imp()->parentLink);

    if (view)
    {
        view->imp()->children.push_back(this);
        imp()->parentLink = std::prev(view->imp()->children.end());

        if (view->scene() != s)
            imp()->sceneChanged(view->scene());
    }
    else
    {
        imp()->damageScene(parentSceneView());

        if (s != nullptr)
            imp()->sceneChanged(nullptr);
    }

    imp()->markAsChangedOrder();
    imp()->parent = view;
}

void LView::insertAfter(LView *prev, bool switchParent)
{
    if (prev == this)
        return;

    // If prev == nullptr, insert to the front of current parent children list
    if (!prev)
    {
        // If no parent, is a no-op
        if (!parent())
            return;

        // Already in front
        if (parent()->children().front() == this)
            return;

        parent()->imp()->children.erase(imp()->parentLink);
        parent()->imp()->children.push_front(this);
        imp()->parentLink = parent()->imp()->children.begin();

        imp()->markAsChangedOrder();

        repaint();
    }
    else
    {
        if (switchParent)
        {
            setParent(prev->parent());
        }
        else
        {
            if (prev->parent() != parent())
                return;
        }

        imp()->markAsChangedOrder();

        repaint();

        if (!parent())
            return;

        if (prev == parent()->children().back())
        {
            parent()->imp()->children.erase(imp()->parentLink);
            parent()->imp()->children.push_back(this);
            imp()->parentLink = std::prev(parent()->imp()->children.end());
        }
        else
        {
            parent()->imp()->children.erase(imp()->parentLink);
            imp()->parentLink = parent()->imp()->children.insert(std::next(prev->imp()->parentLink), this);
        }
    }
}

std::list<Louvre::LView *> &LView::children() const
{
    return imp()->children;
}

bool LView::parentOffsetEnabled() const
{
    return imp()->hasFlag(LVS::ParentOffset);
}

void LView::enableParentOffset(bool enabled)
{
    if (mapped() && enabled != imp()->hasFlag(LVS::ParentOffset))
        repaint();

    imp()->setFlag(LVS::ParentOffset, enabled);
}

const LPoint &LView::pos() const
{
    imp()->tmpPoint = nativePos();

    if (parent())
    {
        if (parentScalingEnabled())
            imp()->tmpPoint *= parent()->scalingVector(parent()->type() == Scene);

        if (parentOffsetEnabled())
            imp()->tmpPoint += parent()->pos();
    }

    return imp()->tmpPoint;
}

const LSize &LView::size() const
{
    imp()->tmpSize = nativeSize();

    if (scalingEnabled())
        imp()->tmpSize *= scalingVector(true);

    if (parent() && parentScalingEnabled())
        imp()->tmpSize *= parent()->scalingVector(parent()->type() == Scene);

    return imp()->tmpSize;
}

bool LView::clippingEnabled() const
{
    return imp()->hasFlag(LVS::Clipping);
}

void LView::enableClipping(bool enabled)
{
    if (imp()->hasFlag(LVS::Clipping) != enabled)
    {
        imp()->setFlag(LVS::Clipping, enabled);
        repaint();
    }
}

const LRect &LView::clippingRect() const
{
    return imp()->clippingRect;
}

void LView::setClippingRect(const LRect &rect)
{
    if (rect != imp()->clippingRect)
    {
        imp()->clippingRect = rect;
        repaint();
    }
}

bool LView::parentClippingEnabled() const
{
    return imp()->hasFlag(LVS::ParentClipping);
}

void LView::enableParentClipping(bool enabled)
{
    if (mapped() && enabled != imp()->hasFlag(LVS::ParentClipping))
        repaint();

    imp()->setFlag(LVS::ParentClipping, enabled);
}

bool LView::scalingEnabled() const
{
    return imp()->hasFlag(LVS::Scaling);
}

void LView::enableScaling(bool enabled)
{
    if (mapped() && enabled != imp()->hasFlag(LVS::Scaling))
        repaint();

    imp()->setFlag(LVS::Scaling, enabled);
}

bool LView::parentScalingEnabled() const
{
    return imp()->hasFlag(LVS::ParentScaling);
}

void LView::enableParentScaling(bool enabled)
{
    if (mapped() && enabled != imp()->hasFlag(LVS::ParentScaling))
        repaint();

    return imp()->setFlag(LVS::ParentScaling, enabled);
}

const LSizeF &LView::scalingVector(bool forceIgnoreParent) const
{
    if (forceIgnoreParent)
        return imp()->scalingVector;

    imp()->tmpPointF = imp()->scalingVector;

    if (parent() && parentScalingEnabled())
        imp()->tmpPointF *= parent()->scalingVector(parent()->type() == Scene);

    return imp()->tmpPointF;
}

void LView::setScalingVector(const LSizeF &scalingVector)
{
    if (mapped() && scalingVector != imp()->scalingVector)
        repaint();

    imp()->scalingVector = scalingVector;
}

bool LView::visible() const
{
    return imp()->hasFlag(LVS::Visible);
}

void LView::setVisible(bool visible)
{
    bool prev = mapped();
    imp()->setFlag(LVS::Visible, visible);

    if (prev != mapped())
        repaint();
}

bool LView::mapped() const
{
    if (type() == Scene && !parent())
        return visible();

    return visible() && nativeMapped() && parent() && parent()->mapped();
}

Float32 LView::opacity(bool forceIgnoreParent) const
{
    if (forceIgnoreParent)
        return imp()->opacity;

    if (parentOpacityEnabled() && parent())
        return imp()->opacity * parent()->opacity(parent()->type() == Scene);

    return imp()->opacity;
}

void LView::setOpacity(Float32 opacity)
{
    if (opacity < 0.f)
        opacity = 0.f;
    else if(opacity > 1.f)
        opacity = 1.f;

    if (mapped() && opacity != imp()->opacity)
        repaint();

    imp()->opacity = opacity;
}

bool LView::parentOpacityEnabled() const
{
    return imp()->hasFlag(LVS::ParentOpacity);
}

void LView::enableParentOpacity(bool enabled)
{
    if (mapped() && imp()->hasFlag(LVS::ParentOpacity) != enabled)
        repaint();

    imp()->setFlag(LVS::ParentOpacity, enabled);
}

bool LView::forceRequestNextFrameEnabled() const
{
    return imp()->hasFlag(LVS::ForceRequestNextFrame);
}

void LView::enableForceRequestNextFrame(bool enabled) const
{
    imp()->setFlag(LVS::ForceRequestNextFrame, enabled);
}

void LView::setBlendFunc(GLenum sRGBFactor, GLenum dRGBFactor, GLenum sAlphaFactor, GLenum dAlphaFactor)
{
    if (imp()->sRGBFactor != sRGBFactor || imp()->dRGBFactor != dRGBFactor ||
        imp()->sAlphaFactor != sAlphaFactor || imp()->dAlphaFactor != dAlphaFactor)
    {
        imp()->sRGBFactor = sRGBFactor;
        imp()->dRGBFactor = dRGBFactor;
        imp()->sAlphaFactor = sAlphaFactor;
        imp()->dAlphaFactor = dAlphaFactor;
        repaint();
    }
}

void LView::enableAutoBlendFunc(bool enabled)
{
    if (enabled == imp()->hasFlag(LVS::AutoBlendFunc))
        return;

    imp()->setFlag(LVS::AutoBlendFunc, enabled);
    repaint();
}

bool LView::autoBlendFuncEnabled() const
{
    return imp()->hasFlag(LVS::AutoBlendFunc);
}

void LView::setColorFactor(Float32 r, Float32 g, Float32 b, Float32 a)
{
    if (imp()->colorFactor.r != r ||
        imp()->colorFactor.g != g ||
        imp()->colorFactor.b != b ||
        imp()->colorFactor.a != a)
    {
        imp()->colorFactor = {r, g, b, a};
        repaint();
        imp()->setFlag(LVS::ColorFactor, r != 1.f || g != 1.f || b != 1.f || a != 1.f);
    }
}

const LRGBAF &LView::colorFactor()
{
    return imp()->colorFactor;
}

bool LView::pointerIsOver() const
{
    return imp()->hasFlag(LVS::PointerIsOver);
}

void LView::enableBlockPointer(bool enabled)
{
    imp()->setFlag(LVS::BlockPointer, enabled);
}

bool LView::blockPointerEnabled() const
{
    return imp()->hasFlag(LVS::BlockPointer);
}

void LView::enableBlockTouch(bool enabled)
{
    imp()->setFlag(LVS::BlockTouch, enabled);
}

bool LView::blockTouchEnabled() const
{
    return imp()->hasFlag(LVS::BlockTouch);
}

LBox LView::boundingBox() const
{
    LBox box =
    {
        pos().x(),
        pos().y(),
        pos().x() + size().w(),
        pos().y() + size().h(),
    };

    LBox childBox;

    for (LView *child : children())
    {
        if (!child->mapped())
            continue;

        childBox = child->boundingBox();

        if (childBox.x1 < box.x1)
            box.x1 = childBox.x1;

        if (childBox.y1 < box.y1)
            box.y1 = childBox.y1;

        if (childBox.x2 > box.x2)
            box.x2 = childBox.x2;

        if (childBox.y2 > box.y2)
            box.y2 = childBox.y2;
    }

    return box;
}
