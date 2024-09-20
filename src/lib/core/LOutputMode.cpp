#include <LOutput.h>
#include <LOutputMode.h>

using namespace Louvre;

LOutputMode::LOutputMode(LOutput *output, const LSize &size, UInt32 refreshRate, bool isPreferred, void *data) noexcept :
    m_sizeB(size),
    m_refreshRate(refreshRate),
    m_output(output),
    m_data(data),
    m_isPreferred(isPreferred)
{}
