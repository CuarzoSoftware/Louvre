#include <private/LLayerViewPrivate.h>
#include <LCompositor.h>

LLayerView::LLayerView(LView *parent) :
    LView(LView::Layer, parent),
    LPRIVATE_INIT_UNIQUE(LLayerView)
{}

LLayerView::~LLayerView()
{
    if (imp()->inputRegion)
        delete imp()->inputRegion;
}

void LLayerView::setPos(Int32 x, Int32 y)
{
    if (mapped() && (x != imp()->nativePos.x() || y != imp()->nativePos.y()))
        repaint();

    imp()->nativePos.setX(x);
    imp()->nativePos.setY(y);
}

void LLayerView::setSize(Int32 w, Int32 h)
{
    if (mapped() && (w != imp()->nativeSize.w() || h != imp()->nativeSize.h()))
        repaint();

    imp()->nativeSize.setW(w);
    imp()->nativeSize.setH(h);
}

void LLayerView::setPos(const LPoint &pos)
{
    setPos(pos.x(), pos.y());
}

void LLayerView::setSize(const LSize &size)
{
    setSize(size.w(), size.h());
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

Float32 LLayerView::bufferScale() const
{
    return 1.f;
}

void LLayerView::enteredOutput(LOutput *output)
{
    LVectorPushBackIfNonexistent(imp()->outputs, output);
}

void LLayerView::leftOutput(LOutput *output)
{
    LVectorRemoveOneUnordered(imp()->outputs, output);
}

const std::vector<LOutput*> &LLayerView::outputs() const
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

void LLayerView::paintEvent(const PaintEventParams &)
{
    /* It is not renderable */
}
