#include "LDRMOutput.h"
#include "SRMConnector.h"
#include <CZ/Louvre/Backends/DRM/LDRMOutputMode.h>

using namespace CZ;

LDRMOutputMode::LDRMOutputMode(SRMConnectorMode *mode) noexcept : m_mode(mode)
{
    m_mode->userData = this;
}

std::shared_ptr<LDRMOutputMode> LDRMOutputMode::Make(SRMConnectorMode *mode) noexcept
{
    auto lmode { std::shared_ptr<LDRMOutputMode>(new LDRMOutputMode(mode)) };
    lmode->m_self = lmode;
    return lmode;
}

SkISize LDRMOutputMode::size() const noexcept
{
    return m_mode ? m_mode->size() : SkISize::MakeEmpty();
}

UInt32 LDRMOutputMode::refreshRate() const noexcept
{
    return m_mode ? m_mode->refreshRate() * 1000 : 0;
}

bool LDRMOutputMode::isPreferred() const noexcept
{
    return m_mode ? m_mode->isPreferred() : false;
}

CZ::LOutput *CZ::LDRMOutputMode::output() const noexcept
{
    return m_mode ? static_cast<LDRMOutput*>(m_mode->connector()->userData)->output() : nullptr;
}
