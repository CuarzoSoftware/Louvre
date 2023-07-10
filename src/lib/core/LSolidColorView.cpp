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

bool LSolidColorView::mapped() const
{
    return visible();
}

const LPoint &LSolidColorView::posC() const
{
    if (parent())
    {
        imp()->tmpPos = imp()->posC + parent()->posC();
        return imp()->tmpPos;
    }
    return imp()->posC;
}

const LSize &LSolidColorView::sizeC() const
{
    return scaledSizeC();
}

Int32 LSolidColorView::scale() const
{
    return 1;
}

void LSolidColorView::enterOutput(LOutput *output)
{
    imp()->outputs.push_back(output);
}

void LSolidColorView::leaveOutput(LOutput *output)
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

bool LSolidColorView::forceRequestNextFrameEnabled() const
{
    return false;
}

void LSolidColorView::enableForceRequestNextFrame(bool enabled)
{
    L_UNUSED(enabled)
}

void LSolidColorView::requestNextFrame(LOutput *output)
{
    L_UNUSED(output);
}

const LRegion *LSolidColorView::damageC() const
{
    return &imp()->damageC;
}

const LRegion *LSolidColorView::translucentRegionC() const
{
    imp()->translucentRegionC.clear();

    if (opacity() <= 1.f)
        imp()->translucentRegionC.addRect(0 ,0, sizeC().w(), sizeC().h());

    return &imp()->translucentRegionC;
}

const LRegion *LSolidColorView::opaqueRegionC() const
{
    imp()->opaqueRegionC.clear();

    if (opacity() >= 1.f)
        imp()->opaqueRegionC.addRect(0 ,0, sizeC().w(), sizeC().h());

    return &imp()->opaqueRegionC;
}

const LRegion *LSolidColorView::inputRegionC() const
{
    return &imp()->inputRegionC;
}

void LSolidColorView::paintRect(LPainter *p, Int32, Int32, Int32, Int32, Int32 dstX, Int32 dstY, Int32 dstW, Int32 dstH, Float32, Float32 alpha)
{
    p->drawColorC(dstX, dstY, dstW, dstH,
                  color().r, color().g, color().b,
                  alpha);
}
