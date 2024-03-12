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

bool LLayerView::nativeMapped() const noexcept
{
    return true;
}

const LPoint &LLayerView::nativePos() const noexcept
{
    return imp()->nativePos;
}

const LSize &LLayerView::nativeSize() const noexcept
{
    return imp()->nativeSize;
}

Float32 LLayerView::bufferScale() const noexcept
{
    return 1.f;
}

void LLayerView::enteredOutput(LOutput *output) noexcept
{
    LVectorPushBackIfNonexistent(imp()->outputs, output);
}

void LLayerView::leftOutput(LOutput *output) noexcept
{
    LVectorRemoveOneUnordered(imp()->outputs, output);
}

const std::vector<LOutput*> &LLayerView::outputs() const noexcept
{
    return imp()->outputs;
}

bool LLayerView::isRenderable() const noexcept
{
    return false;
}

void LLayerView::requestNextFrame(LOutput *output) noexcept
{
    L_UNUSED(output);
}

const LRegion *LLayerView::damage() const noexcept
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::translucentRegion() const noexcept
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::opaqueRegion() const noexcept
{
    return &imp()->dummyRegion;
}

const LRegion *LLayerView::inputRegion() const noexcept
{
    return imp()->inputRegion;
}

void LLayerView::paintEvent(const PaintEventParams &) noexcept
{
    /* It is not renderable */
}
