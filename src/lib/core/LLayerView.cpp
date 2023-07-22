#include <private/LLayerViewPrivate.h>
#include <LCompositor.h>

LLayerView::LLayerView(LView *parent) : LView(LView::Layer, parent)
{
    m_imp = new LLayerViewPrivate();
}

LLayerView::~LLayerView()
{
    if (imp()->inputRegion)
        delete imp()->inputRegion;

    delete m_imp;
}

void LLayerView::setPos(const LPoint &pos)
{
    if (mapped() && pos != imp()->nativePos)
        repaint();

    imp()->nativePos = pos;
}

void LLayerView::setSize(const LSize &size)
{
    if (mapped() && size != imp()->nativeSize)
        repaint();

    imp()->nativeSize = size;
}

void LLayerView::setInputRegion(const LRegion *region) const
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

const LPoint &LLayerView::nativePos() const
{
    return imp()->nativePos;
}

const LSize &LLayerView::nativeSize() const
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

const LRegion *LLayerView::damage() const
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::translucentRegion() const
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::opaqueRegion() const
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::inputRegion() const
{
    return imp()->inputRegion;
}

void LLayerView::paintRect(LPainter *,
                            Int32, Int32, Int32, Int32,
                            Int32, Int32, Int32, Int32,
                            Float32, Float32)
{
    /* It is not renderable */
}
