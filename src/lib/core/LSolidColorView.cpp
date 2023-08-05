#include <private/LSolidColorViewPrivate.h>
#include <LPainter.h>

Louvre::LSolidColorView::LSolidColorView(LView *parent) : LView(LView::SolidColor, parent)
{
    m_imp = new LSolidColorViewPrivate();
    imp()->color = {0, 0, 0};
    setOpacity(1.f);
}

Louvre::LSolidColorView::LSolidColorView(Float32 r, Float32 g, Float32 b, Float32 a, LView *parent) : LView(SolidColor, parent)
{
    m_imp = new LSolidColorViewPrivate();
    imp()->color = {r, g, b};
    setOpacity(a);
}

LSolidColorView::LSolidColorView(const LRGBF &color, Float32 a, LView *parent) : LView(SolidColor, parent)
{
    m_imp = new LSolidColorViewPrivate();
    imp()->color = color;
    setOpacity(a);
}

LSolidColorView::~LSolidColorView()
{
    delete m_imp;
}

void LSolidColorView::setColor(const LRGBF &color)
{
    imp()->color = color;
    repaint();
}

void LSolidColorView::setColor(Float32 r, Float32 g, Float32 b)
{
    imp()->color = {r,g,b};
    repaint();
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

Int32 LSolidColorView::bufferScale() const
{
    return 0;
}

void LSolidColorView::enteredOutput(LOutput *output)
{
    imp()->outputs.remove(output);
    imp()->outputs.push_back(output);
}

void LSolidColorView::leftOutput(LOutput *output)
{
    imp()->outputs.remove(output);
}

const std::list<LOutput *> &LSolidColorView::outputs() const
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

void LSolidColorView::paintRect(LPainter *p,
                                 Int32, Int32, Int32, Int32,
                                 Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH,
                                 Float32, Float32 alpha)
{
    p->drawColor(dstX, dstY, dstW, dstH,
                  color().r, color().g, color().b,
                  alpha);
}
