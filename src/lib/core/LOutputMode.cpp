#include <private/LOutputModePrivate.h>
#include <private/LCompositorPrivate.h>
#include <LOutput.h>

using namespace Louvre;

LOutputMode::LOutputMode(LOutput *output) noexcept :
    LPRIVATE_INIT_UNIQUE(LOutputMode)
{
    imp()->output = output;
}

LOutput *LOutputMode::output() const
{
    return imp()->output;
}

LOutputMode::~LOutputMode() {}

const LSize &LOutputMode::sizeB() const
{
    return *compositor()->imp()->graphicBackend->outputModeGetSize((LOutputMode*)this);
}

UInt32 LOutputMode::refreshRate() const
{
    return compositor()->imp()->graphicBackend->outputModeGetRefreshRate((LOutputMode*)this);
}

bool LOutputMode::isPreferred() const
{
    return compositor()->imp()->graphicBackend->outputModeIsPreferred((LOutputMode*)this);
}
