#include <private/LLayerViewPrivate.h>
#include <LCompositor.h>

LLayerView::LLayerView(LView *parent) : LView(LView::Layer, parent)
{
    m_imp = new LLayerViewPrivate();
    enableInput(false);
}

LLayerView::~LLayerView()
{
    if (imp()->inputRegion)
        delete imp()->inputRegion;

    delete m_imp;
}

void LLayerView::setNativePosC(const LPoint &pos)
{
    if (mapped() && pos != imp()->nativePos)
        repaint();

    imp()->nativePos = pos;
}

void LLayerView::setNativeSizeC(const LSize &size)
{
    if (mapped() && size != imp()->nativeSize)
        repaint();

    imp()->nativeSize = size;
}

void LLayerView::setInputRegionC(const LRegion *region) const
{
    if (region)
    {
        if (imp()->inputRegion)
            *imp()->inputRegion = *region;
        else
        {
            imp()->inputRegion = new LRegion();
            *imp()->inputRegion = *region;
        }
    }
    else
    {
        if (imp()->inputRegion)
        {
            delete imp()->inputRegion;
            imp()->inputRegion = nullptr;
        }
    }
}

bool LLayerView::nativeMapped() const
{
    return true;
}

const LPoint &LLayerView::nativePosC() const
{
    return imp()->nativePos;
}

const LSize &LLayerView::nativeSizeC() const
{
    return imp()->nativeSize;
}

Int32 LLayerView::bufferScale() const
{
    return 0;
}

void LLayerView::enteredOutput(LOutput *output)
{
    imp()->outputs.remove(output);
    imp()->outputs.push_back(output);
}

void LLayerView::leftOutput(LOutput *output)
{
    imp()->outputs.remove(output);
}

const std::list<LOutput*> &LLayerView::outputs() const
{
    return imp()->outputs;
}

bool LLayerView::isRenderable() const
{
    return false;
}

void LLayerView::requestNextFrame(LOutput *output)
{
    L_UNUSED(output);
}

const LRegion *LLayerView::damageC() const
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::translucentRegionC() const
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::opaqueRegionC() const
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::inputRegionC() const
{
    return imp()->inputRegion;
}

void LLayerView::paintRectC(LPainter *, Int32, Int32, Int32, Int32, Int32, Int32, Int32, Int32, Float32, Float32)
{
    /* It is not renderable */
}
