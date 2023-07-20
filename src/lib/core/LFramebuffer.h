#ifndef LFRAMEBUFFER_H
#define LFRAMEBUFFER_H

#include <LObject.h>

class Louvre::LFramebuffer : public LObject
{
public:
    virtual ~LFramebuffer(){};
    virtual Int32 scale() const = 0;
    virtual const LSize &sizeB() const = 0;
    virtual const LRect &rect() const = 0;
    virtual GLuint id(LOutput *output) const = 0;
    virtual Int32 buffersCount() const = 0;
    virtual Int32 currentBufferIndex() const = 0;
    virtual const LTexture *texture(Int32 index = 0) const = 0;
    virtual void setFramebufferDamageC(const LRegion *damage) = 0;
};

#endif // LFRAMEBUFFER_H
