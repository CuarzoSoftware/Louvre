#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LCompositorPrivate.h>
#include <LOutputFramebuffer.h>

using namespace Louvre;

Float32 LOutputFramebuffer::scale() const noexcept
{
    return m_output->imp()->scale;
}

SkISize LOutputFramebuffer::sizeB() const noexcept
{
    return m_output->imp()->sizeB;
}

const SkIRect &LOutputFramebuffer::rect() const noexcept
{
    return m_output->imp()->rect;
}

GLuint LOutputFramebuffer::id() const noexcept
{
    if (m_output->usingFractionalScale() && m_output->fractionalOversamplingEnabled())
        return m_output->imp()->fractionalFb.id();

    return compositor()->imp()->graphicBackend->outputGetFramebufferID(m_output);
}

Int32 LOutputFramebuffer::bufferAge() const noexcept
{
    return m_output->currentBufferAge();
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

void LOutputFramebuffer::setFramebufferDamage(const SkRegion *damage) noexcept
{
    m_output->setBufferDamage(damage);
}

CZTransform LOutputFramebuffer::transform() const noexcept
{
    return m_output->imp()->transform;
}
