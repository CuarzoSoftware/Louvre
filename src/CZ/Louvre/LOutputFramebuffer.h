#ifndef LOUTPUTFRAMEBUFFER_H
#define LOUTPUTFRAMEBUFFER_H

#include <CZ/Louvre/LFramebuffer.h>

/**
 * @brief An output framebuffer
 *
 * An LOutputFramebuffer is a specific type of framebuffer used by an LOutput and managed by the graphic backend.
 *
 * @note This class is primarily used internally by Louvre and may have limited utility for library users.
 *
 * It's advisable to use the interface provided by LOutput instead of directly accessing the methods associated with this class.
 *
 * @see LOutput::framebuffer()
 * @see LOutput::currentBuffer()
 * @see LOutput::buffersCount()
 * @see LOutput::bufferTexture()
 * @see LOutput::hasBufferDamageSupport()
 * @see LOutput::setBufferDamage()
 */
class Louvre::LOutputFramebuffer final : public LFramebuffer
{
public:
    LCLASS_NO_COPY(LOutputFramebuffer)

    LOutput *output() const noexcept
    {
        return m_output;
    }

    Float32 scale() const noexcept override;
    SkISize sizeB() const noexcept override;
    const SkIRect &rect() const noexcept override;
    GLuint id() const noexcept override;
    Int32 bufferAge() const noexcept override;
    Int32 buffersCount() const noexcept override;
    Int32 currentBufferIndex() const noexcept override;
    LTexture *texture(Int32 index = 0) const noexcept override;
    void setFramebufferDamage(const SkRegion *damage) noexcept override;
    CZTransform transform() const noexcept override;

private:
    friend class LOutput;
    LOutputFramebuffer(LOutput *output) noexcept : LFramebuffer(Output), m_output(output) {}
    ~LOutputFramebuffer() { notifyDestruction(); };
    LOutput *m_output;
};

#endif // LOUTPUTFRAMEBUFFER_H
