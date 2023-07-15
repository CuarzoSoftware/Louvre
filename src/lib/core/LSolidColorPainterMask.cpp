#include <private/LSolidColorPainterMaskPrivate.h>
#include <private/LPainterPrivate.h>

LSolidColorPainterMask::LSolidColorPainterMask(const LRGBAF &color,
                                               const LRect &rect,
                                               BlendMode mode)
: LPainterMask(LPainterMask::SolidColor, mode, rect)
{
    m_imp = new LSolidColorPainterMaskPrivate();
    imp()->color = color;
}

LSolidColorPainterMask::LSolidColorPainterMask(Float32 r, Float32 g, Float32 b, Float32 a,
                                               const LRect &rect, BlendMode mode)
: LPainterMask(LPainterMask::SolidColor, mode, rect)
{
    m_imp = new LSolidColorPainterMaskPrivate();
    imp()->color = {r, g, b, a};
}

LSolidColorPainterMask::~LSolidColorPainterMask()
{
    delete m_imp;
}

const LRGBAF &LSolidColorPainterMask::color() const
{
    return imp()->color;
}

void LSolidColorPainterMask::setColor(const LRGBAF &color)
{
    imp()->color = color;
}

void LSolidColorPainterMask::setColor(Float32 r, Float32 g, Float32 b, Float32 a)
{
    imp()->color = {r, g, b, a};
}

bool LSolidColorPainterMask::bind(LPainter *painter, UInt32 unit)
{
    painter->imp()->masksTypes[unit] = type();
    painter->imp()->masksModes[unit] = blendMode();
    painter->imp()->masksRects[unit].x = rectC().x();
    painter->imp()->masksRects[unit].y = rectC().y();
    painter->imp()->masksRects[unit].w = rectC().w();
    painter->imp()->masksRects[unit].h = rectC().h();
    painter->imp()->masksColors[unit].x = color().r;
    painter->imp()->masksColors[unit].y = color().g;
    painter->imp()->masksColors[unit].w = color().b;
    painter->imp()->masksColors[unit].h = color().a;
    return true;
}
