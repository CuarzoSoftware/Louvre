#include <private/LPainterMaskPrivate.h>

LPainterMask::LPainterMask(UInt32 type, BlendMode mode, const LRect &rectC)
{
    m_imp = new LPainterMaskPrivate();
    imp()->type = type;
    imp()->blendMode = mode;
    imp()->rect = rectC;
}

LPainterMask::~LPainterMask()
{
    delete m_imp;
}

UInt32 LPainterMask::type() const
{
    return imp()->type;
}

LPainterMask::BlendMode LPainterMask::blendMode() const
{
    return imp()->blendMode;
}

void LPainterMask::setBlendMode(BlendMode mode)
{
    imp()->blendMode = mode;
}

const LRect &LPainterMask::rectC() const
{
    return imp()->rect;
}

void LPainterMask::setRectC(const LRect &rect)
{
    imp()->rect = rect;
}
