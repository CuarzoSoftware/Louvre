#include <private/LSolidColorViewPrivate.h>
#include <private/LPainterPrivate.h>
#include <private/LViewPrivate.h>

Louvre::LSolidColorView::LSolidColorView(LView *parent) :
    LView(LView::SolidColor, parent),
    LPRIVATE_INIT_UNIQUE(LSolidColorView)
{
    imp()->color = {0, 0, 0};
    LView *nativeView = (LView*)this;
    nativeView->imp()->opacity = 1.f;
}

Louvre::LSolidColorView::LSolidColorView(Float32 r, Float32 g, Float32 b, Float32 a, LView *parent) :
    LView(SolidColor, parent),
    LPRIVATE_INIT_UNIQUE(LSolidColorView)
{
    imp()->color = {r, g, b};

    if (a < 0.f)
        a = 0.f;
    else if(a > 1.f)
        a = 1.f;

    LView *nativeView = (LView*)this;
    nativeView->imp()->opacity = a;
}

LSolidColorView::LSolidColorView(const LRGBF &color, Float32 a, LView *parent) :
    LView(SolidColor, parent),
    LPRIVATE_INIT_UNIQUE(LSolidColorView)
{
    imp()->color = color;

    if (a < 0.f)
        a = 0.f;
    else if(a > 1.f)
        a = 1.f;

    LView *nativeView = (LView*)this;
    nativeView->imp()->opacity = a;
}

LSolidColorView::~LSolidColorView() {}

void LSolidColorView::setColor(const LRGBF &color)
{
    setColor(color.r, color.g, color.b);
}

void LSolidColorView::setColor(Float32 r, Float32 g, Float32 b)
{
    if (imp()->color.r != r || imp()->color.g != g || imp()->color.b != b)
    {
        imp()->color = {r, g, b};
        LView *nativeView = this;
        nativeView->imp()->markAsChangedOrder(false);
        repaint();
    }
}

const LRGBF &LSolidColorView::color() const
{
    return imp()->color;
}

void LSolidColorView::setPos(const LPoint &pos)
{
    setPos(pos.x(), pos.y());
}

void LSolidColorView::setPos(Int32 x, Int32 y)
{
    if (mapped() && (x != imp()->nativePos.x() || y != imp()->nativePos.y()))
        repaint();

    imp()->nativePos.setX(x);
    imp()->nativePos.setY(y);
}

void LSolidColorView::setSize(const LSize &size)
{
    setSize(size.w(), size.h());
}

void LSolidColorView::setSize(Int32 w, Int32 h)
{
    if (w != imp()->nativeSize.w() || h != imp()->nativeSize.h())
    {
        imp()->nativeSize.setW(w);
        imp()->nativeSize.setH(h);

        imp()->opaqueRegion.clear();
        imp()->opaqueRegion.addRect(LRect(LPoint(0,0), imp()->nativeSize));

        if (mapped())
            repaint();
    }
}

void LSolidColorView::setInputRegion(const LRegion *region) const
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

bool LSolidColorView::nativeMapped() const
{
    return true;
}

const LPoint &LSolidColorView::nativePos() const
{
    return imp()->nativePos;
}

const LSize &LSolidColorView::nativeSize() const
{
    return imp()->nativeSize;
}

Float32 LSolidColorView::bufferScale() const
{
    return 1.f;
}

void LSolidColorView::enteredOutput(LOutput *output)
{
    LVectorPushBackIfNonexistent(imp()->outputs, output);
}

void LSolidColorView::leftOutput(LOutput *output)
{
    LVectorRemoveOneUnordered(imp()->outputs, output);
}

const std::vector<LOutput *> &LSolidColorView::outputs() const
{
    return imp()->outputs;
}

bool LSolidColorView::isRenderable() const
{
    return true;
}

void LSolidColorView::requestNextFrame(LOutput *output)
{
    L_UNUSED(output);
}

const LRegion *LSolidColorView::damage() const
{
    return &imp()->emptyRegion;
}

const LRegion *LSolidColorView::translucentRegion() const
{
    return &imp()->emptyRegion;
}

const LRegion *LSolidColorView::opaqueRegion() const
{
    return &imp()->opaqueRegion;
}

const LRegion *LSolidColorView::inputRegion() const
{
    return imp()->inputRegion;
}

void LSolidColorView::paintEvent(const PaintEventParams &params)
{
    params.painter->setColor(color());
    params.painter->bindColorMode();
    params.painter->drawRegion(*params.region);
}
