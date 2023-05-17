#include <private/LOutputModePrivate.h>
#include <private/LCompositorPrivate.h>
#include <LOutput.h>

using namespace Louvre;

LOutputMode::LOutputMode(const LOutput *output)
{
    m_imp = new LOutputModePrivate();
    m_imp->output = output;
}

const LOutput *LOutputMode::output() const
{
    return m_imp->output;
}

LOutputMode::~LOutputMode()
{
    delete m_imp;
}

const LSize &LOutputMode::sizeB() const
{
    return *output()->compositor()->imp()->graphicBackend->getOutputModeSize((LOutputMode*)this);
}

UInt32 LOutputMode::refreshRate() const
{
    return output()->compositor()->imp()->graphicBackend->getOutputModeRefreshRate((LOutputMode*)this);
}

bool LOutputMode::isPreferred() const
{
    return output()->compositor()->imp()->graphicBackend->getOutputModeIsPreferred((LOutputMode*)this);
}

LOutputMode::LOutputModePrivate *LOutputMode::imp() const
{
    return m_imp;
}
