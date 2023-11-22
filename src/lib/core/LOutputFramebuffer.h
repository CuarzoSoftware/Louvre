#ifndef LOUTPUTFRAMEBUFFER_H
#define LOUTPUTFRAMEBUFFER_H

#include <LFramebuffer.h>

/**
 * @brief An output framebuffer
 *
 * An LOutputFramebuffer is a specific type of framebuffer used by an LOutput and managed by the graphic backend.
 *
 * @note This class is primarily used internally by Louvre and may have limited utility for library users.
 *
 * It is recommended to use the interface provided by LOutput rather than accessing these methods directly.
 *
 * @see LOutput::framebuffer()
 * @see LOutput::currentBuffer()
 * @see LOutput::buffersCount()
 * @see LOutput::bufferTexture()
 * @see LOutput::hasBufferDamageSupport()
 * @see LOutput::setBufferDamage()
 */
class Louvre::LOutputFramebuffer : public LFramebuffer
{
public:
    /// @cond OMIT
    LOutputFramebuffer(LOutput *output);
    ~LOutputFramebuffer();
    /// @endcond

    Int32 scale() const override;
    const LSize &sizeB() const override;
    const LRect &rect() const override;
    GLuint id() const override;
    Int32 buffersCount() const override;
    Int32 currentBufferIndex() const override;
    const LTexture *texture(Int32 index = 0) const override;
    void setFramebufferDamage(const LRegion *damage) override;
    Transform transform() const override;

    LPRIVATE_IMP_UNIQUE(LOutputFramebuffer)
};

#endif // LOUTPUTFRAMEBUFFER_H
