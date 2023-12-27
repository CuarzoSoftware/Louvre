#include <private/LCompositorPrivate.h>
#include <private/LViewPrivate.h>
#include <private/LScenePrivate.h>
#include <private/LSceneViewPrivate.h>
#include <LOutput.h>
#include <LLog.h>
#include <string.h>

using namespace Louvre;

using LVS = LView::LViewPrivate::LViewState;

LView::LView(UInt32 type, LView *parent) : LPRIVATE_INIT_UNIQUE(LView)
{
    imp()->type = type;
    compositor()->imp()->views.push_back(this);
    imp()->compositorLink = std::prev(compositor()->imp()->views.end());
    setParent(parent);
}

LView::~LView()
{
    setParent(nullptr);

    while (!children().empty())
        children().front()->setParent(nullptr);

    compositor()->imp()->views.erase(imp()->compositorLink);
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

    for (std::list<LOutput*>::const_iterator it = outputs().cbegin(); it != outputs().cend(); it++)
        (*it)->repaint();

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

    LScene *s = scene();

    if (s)
        s->imp()->listChanged = true;

    if (parent())
        parent()->imp()->children.erase(imp()->parentLink);

    if (view)
    {
        view->imp()->children.push_back(this);
        imp()->parentLink = std::prev(view->imp()->children.end());
    }
    else
    {
        imp()->damageScene(parentSceneView());
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

bool LView::inputEnabled() const
{
    return imp()->hasFlag(LVS::Input);
}

void LView::enableInput(bool enabled)
{
    imp()->setFlag(LVS::Input, enabled);
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

void LView::pointerEnterEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);
}

void LView::pointerMoveEvent(const LPoint &localPos)
{
    L_UNUSED(localPos);
}

void LView::pointerLeaveEvent()
{
}

void LView::pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state)
{
    L_UNUSED(button);
    L_UNUSED(state);
}

void LView::pointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source)
{
    L_UNUSED(axisX);
    L_UNUSED(axisY);
    L_UNUSED(discreteX);
    L_UNUSED(discreteY);
    L_UNUSED(source);
}

void LView::keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    L_UNUSED(depressed);
    L_UNUSED(latched);
    L_UNUSED(locked);
    L_UNUSED(group);
}

void LView::keyEvent(UInt32 keyCode, UInt32 keyState)
{
    L_UNUSED(keyCode);
    L_UNUSED(keyState);
}
