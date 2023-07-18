#ifndef LRENDERBUFFER_H
#define LRENDERBUFFER_H

#include <LFramebuffer.h>

class Louvre::LRenderBuffer : public LFramebuffer
{
public:
    LRenderBuffer(const LSize &sizeB);
    ~LRenderBuffer();

    void setSizeB(const LSize &sizeB);
    void setScale(Int32 scale) const;
    const LSize &sizeC() const;
    const LPoint &posC() const;
    void setPosC(const LPoint &pos);

    Int32 scale() const override;
    const LSize &sizeB() const override;
    const LRect &rectC() const override;
    GLuint id(LOutput *output) const override;
    Int32 buffersCount() const override;
    Int32 currentBufferIndex() const override;
    const LTexture *texture(Int32 index = 0) const override;
    void setFramebufferDamageC(const LRegion *damage) override;

LPRIVATE_IMP(LRenderBuffer)
};

#endif // LRENDERBUFFER_H