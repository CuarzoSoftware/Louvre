#include <private/LOutputFramebufferPrivate.h>
#include <LOutput.h>
#include <GLES2/gl2.h>

LOutputFramebuffer::LOutputFramebuffer(LOutput *output) :
LPRIVATE_INIT_UNIQUE(LOutputFramebuffer)
{
    imp()->output = output;
}

LOutputFramebuffer::~LOutputFramebuffer() {}

Int32 LOutputFramebuffer::scale() const
{
    return imp()->output->scale();
}

const LSize &LOutputFramebuffer::sizeB() const
{
    return imp()->output->sizeB();
}

const LRect &LOutputFramebuffer::rect() const
{
    return imp()->output->rect();
}

GLuint LOutputFramebuffer::id() const
{
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
    imp()->output->setBufferDamage(*damage);
}

LFramebuffer::Transform LOutputFramebuffer::transform() const
{
    return imp()->output->transform();
}
