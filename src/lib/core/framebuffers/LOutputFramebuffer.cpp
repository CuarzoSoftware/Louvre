#include <private/LOutputPrivate.h>
#include <private/LCompositorPrivate.h>
#include <LOutputFramebuffer.h>

using namespace Louvre;

Float32 LOutputFramebuffer::scale() const noexcept
{
    return m_output->imp()->scale;
}

const LSize &LOutputFramebuffer::sizeB() const noexcept
{
    return m_output->imp()->sizeB;
}

const LRect &LOutputFramebuffer::rect() const noexcept
{
    return m_output->imp()->rect;
}

GLuint LOutputFramebuffer::id() const noexcept
{
    if (m_output->usingFractionalScale() && m_output->fractionalOversamplingEnabled())
        return m_output->imp()->fractionalFb.id();

    return compositor()->imp()->graphicBackend->outputGetFramebufferID(m_output);
}

Int32 LOutputFramebuffer::buffersCount() const noexcept
{
    return m_output->buffersCount();
}

Int32 LOutputFramebuffer::currentBufferIndex() const noexcept
{
    return m_output->currentBuffer();
}

LTexture *LOutputFramebuffer::texture(Int32 index) const noexcept
{
    return m_output->bufferTexture(index);
}

void LOutputFramebuffer::setFramebufferDamage(const LRegion *damage) noexcept
{
    m_output->setBufferDamage(damage);
}

LTransform LOutputFramebuffer::transform() const noexcept
{
    return m_output->imp()->transform;
}
