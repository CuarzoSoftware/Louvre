#include <private/LLayerViewPrivate.h>
#include <LCompositor.h>

LLayerView::LLayerView(LView *parent) : LView(LView::Layer, parent)
{
    m_imp = new LLayerViewPrivate();
}

LLayerView::~LLayerView()
{
    delete m_imp;
}

const LPoint &LLayerView::customPosC() const
{
    return imp()->customPosC;
}

void LLayerView::setCustomSize(const LSize &size)
{
    imp()->customSizeC = size;

    if (clippingEnabled())
        repaint();
}

void LLayerView::setCustomPosC(const LPoint &pos)
{
    imp()->customPosC = pos;
    repaint();
}

void LLayerView::setInputRegionC(const LRegion &region) const
{
    imp()->inputRegionC = region;
}

const LRegion &Louvre::LLayerView::inputRegionC() const
{
    return imp()->inputRegionC;
}

bool LLayerView::mapped() const
{
    if (parent())
        return parent()->mapped() && visible();
    return visible();
}

const LPoint &LLayerView::posC() const
{
    if (parent())
    {
        imp()->tmpPosC = parent()->posC() + imp()->customPosC;
        return imp()->tmpPosC;
    }
    return imp()->customPosC;
}

const LSize &LLayerView::sizeC() const
{
    return imp()->customSizeC;
}

Int32 LLayerView::scale() const
{
    return LCompositor::compositor()->globalScale();
}

void LLayerView::enterOutput(LOutput *output)
{
    imp()->outputs.push_back(output);
}

void LLayerView::leaveOutput(LOutput *output)
{
    imp()->outputs.remove(output);
}

const std::list<LOutput *> &LLayerView::outputs() const
{
    return imp()->outputs;
}

bool LLayerView::isRenderable() const
{
    return false;
}

bool LLayerView::forceRequestNextFrameEnabled() const
{
    return false;
}

void LLayerView::enableForceRequestNextFrame(bool enabled)
{
    L_UNUSED(enabled)
}

void LLayerView::requestNextFrame(LOutput *output)
{
    L_UNUSED(output);
}

const LRegion *LLayerView::damageC() const
{
    return nullptr;
}

const LRegion *LLayerView::translucentRegionC() const
{
    return nullptr;
}

const LRegion *LLayerView::opaqueRegionC() const
{
    return nullptr;
}
