#ifndef LOUTPUTFRAMEBUFFER_H
#define LOUTPUTFRAMEBUFFER_H

#include <LFramebuffer.h>

/**
 * @brief Output Framebuffer
 *
 * @note This class is primarily used internally by Louvre and may have limited utility for library users.
 */
class Louvre::LOutputFramebuffer : public LFramebuffer
{
public:
    LOutputFramebuffer(LOutput *output);
    ~LOutputFramebuffer();
    Int32 scale() const override;
    const LSize &sizeB() const override;
    const LRect &rect() const override;
    GLuint id() const override;
    Int32 buffersCount() const override;
    Int32 currentBufferIndex() const override;
    const LTexture *texture(Int32 index = 0) const override;
    void setFramebufferDamage(const LRegion *damage) override;

LPRIVATE_IMP(LOutputFramebuffer)
};

#endif // LOUTPUTFRAMEBUFFER_H
