#include <private/LOutputFramebufferPrivate.h>
#include <LOutput.h>
#include <GLES2/gl2.h>

LOutputFramebuffer::LOutputFramebuffer(LOutput *output)
{
    m_imp = new LOutputFramebufferPrivate();
    imp()->output = output;
}

LOutputFramebuffer::~LOutputFramebuffer()
{
    delete m_imp;
}

Int32 LOutputFramebuffer::scale() const
{
    return imp()->output->scale();
}

const LSize &LOutputFramebuffer::sizeB() const
{
    return imp()->output->sizeB();
}

const LRect &LOutputFramebuffer::rectC() const
{
    return imp()->output->rectC();
}

GLuint LOutputFramebuffer::id(LOutput *output) const
{
    L_UNUSED(output);
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

void LOutputFramebuffer::setFramebufferDamageC(const LRegion *damage)
{
    imp()->output->setBufferDamageC(*damage);
}