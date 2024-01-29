#include <private/LOutputFramebufferPrivate.h>
#include <private/LOutputPrivate.h>

LOutputFramebuffer::LOutputFramebuffer(LOutput *output) :
LPRIVATE_INIT_UNIQUE(LOutputFramebuffer)
{
    m_type = Output;
    imp()->output = output;
}

LOutputFramebuffer::~LOutputFramebuffer() {}

LOutput *LOutputFramebuffer::output() const
{
    return imp()->output;
}

Float32 LOutputFramebuffer::scale() const
{
    return imp()->output->imp()->scale;
}

const LSize &LOutputFramebuffer::sizeB() const
{
    return imp()->output->imp()->sizeB;
}

const LRect &LOutputFramebuffer::rect() const
{
    return imp()->output->imp()->rect;
}

GLuint LOutputFramebuffer::id() const
{
    if (imp()->output->usingFractionalScale() && imp()->output->fractionalOversamplingEnabled())
        return imp()->output->imp()->fractionalFb->id();

    return 0;
}

Int32 LOutputFramebuffer::buffersCount() const
{
    return imp()->output->buffersCount();
}

Int32 LOutputFramebuffer::currentBufferIndex() const
{
    return imp()->output->currentBuffer();
}

const LTexture *LOutputFramebuffer::texture(Int32 index) const
{
    return imp()->output->bufferTexture(index);
}

void LOutputFramebuffer::setFramebufferDamage(const LRegion *damage)
{
    imp()->output->setBufferDamage(damage);
}

LFramebuffer::Transform LOutputFramebuffer::transform() const
{
    return imp()->output->imp()->transform;
}
