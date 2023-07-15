#ifndef LPAINTERMASK_H
#define LPAINTERMASK_H

#include <LObject.h>
#include <LRect.h>

class Louvre::LPainterMask : public LObject
{
public:
    enum Type : UInt32
    {
        SolidColor = 0,
        Texture = 1
    };

    enum BlendMode : UInt32
    {
        ReplaceAlpha = 0
    };

    LPainterMask(UInt32 type, BlendMode mode = ReplaceAlpha, const LRect &rectC = LRect());
    virtual ~LPainterMask();

    UInt32 type() const;

    BlendMode blendMode() const;
    void setBlendMode(BlendMode mode);

    const LRect &rectC() const;
    void setRectC(const LRect &rect);

    virtual bool bind(LPainter *painter, UInt32 unit) = 0;

LPRIVATE_IMP(LPainterMask)
};

#endif // LPAINTERMASK_H
