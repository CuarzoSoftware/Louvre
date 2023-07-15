#ifndef LSOLIDCOLORPAINTERMASK_H
#define LSOLIDCOLORPAINTERMASK_H

#include <LPainterMask.h>

class Louvre::LSolidColorPainterMask : public LPainterMask
{
public:
    LSolidColorPainterMask(const LRGBAF &color,
                           const LRect &rect = LRect(),
                           BlendMode mode = ReplaceAlpha);

    LSolidColorPainterMask(Float32 r, Float32 g, Float32 b, Float32 a,
                           const LRect &rect = LRect(),
                           BlendMode mode = ReplaceAlpha);

    virtual ~LSolidColorPainterMask();

    const LRGBAF &color() const;
    void setColor(const LRGBAF &color);
    void setColor(Float32 r, Float32 g, Float32 b, Float32 a);

    virtual bool bind(LPainter *painter, UInt32 unit) override;

LPRIVATE_IMP(LSolidColorPainterMask)
};

#endif // LSOLIDCOLORPAINTERMASK_H
