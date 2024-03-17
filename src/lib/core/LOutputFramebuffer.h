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
class Louvre::LOutputFramebuffer final : public LFramebuffer
{
public:
    /// @cond OMIT
    inline LOutputFramebuffer(LOutput *output) noexcept : m_output(output)
    {
        m_type = Output;
    }
    ~LOutputFramebuffer() noexcept = default;
    /// @endcond

    inline LOutput *output() const noexcept
    {
        return m_output;
    }

    Float32 scale() const noexcept override;
    const LSize &sizeB() const noexcept override;
    const LRect &rect() const noexcept override;
    GLuint id() const noexcept override;
    Int32 buffersCount() const noexcept override;
    Int32 currentBufferIndex() const noexcept override;
    const LTexture *texture(Int32 index = 0) const noexcept override;
    void setFramebufferDamage(const LRegion *damage) noexcept override;
    Transform transform() const noexcept override;

private:
    LOutput *m_output;
};

#endif // LOUTPUTFRAMEBUFFER_H
