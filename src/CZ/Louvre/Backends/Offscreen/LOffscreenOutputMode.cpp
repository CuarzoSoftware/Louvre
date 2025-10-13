#include <CZ/Louvre/Backends/Offscreen/LOffscreenOutputMode.h>
#include <CZ/Louvre/Backends/Offscreen/LOffscreenOutput.h>

using namespace CZ;

LOffscreenOutputMode::LOffscreenOutputMode(LOffscreenOutput *output, SkISize size) noexcept :
    m_size(size), m_output(output)
{}

LOutput *LOffscreenOutputMode::output() const noexcept
{
    return m_output->output();
}
