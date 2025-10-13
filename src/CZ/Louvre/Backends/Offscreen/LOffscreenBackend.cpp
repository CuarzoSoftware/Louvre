#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/Backends/Offscreen/LOffscreenOutputMode.h>
#include <CZ/Louvre/Backends/Offscreen/LOffscreenBackend.h>
#include <CZ/Louvre/Backends/Offscreen/LOffscreenOutput.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Ream/OF/ROFPlatformHandle.h>
#include <CZ/Ream/RCore.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;

LOutput *LOffscreenBackend::addOutput(SkISize bufferSize) noexcept
{
    if (bufferSize.isEmpty())
        return {};

    LOutput::Params params {};
    params.backend.reset(new LOffscreenOutput(this, bufferSize));
    auto *output { LFactory::createObject<LOutput>(&params) };
    m_outputs.emplace_back(output);

    if (m_initialized)
        seat()->imp()->handleOutputPlugged(output);

    return output;
}

void LOffscreenBackend::removeOutput(LOutput *output) noexcept
{
    CZVectorUtils::RemoveOneUnordered(m_outputs, output);

    if (m_initialized)
        seat()->imp()->handleOutputUnplugged(output);

    delete output;
}

bool LOffscreenBackend::init() noexcept
{
    RCore::Options options {};
    options.graphicsAPI = RGraphicsAPI::RS;
    options.platformHandle = ROFPlatformHandle::Make();

    m_ream = RCore::Make(options);

    if (!m_ream)
    {
        LLog(CZFatal, CZLN, "Failed to create RCore");
        return false;
    }

    addOutput({1024, 1024});
    m_initialized = true;
    return true;
}

void LOffscreenBackend::unit() noexcept
{
    while (!m_outputs.empty())
        removeOutput(m_outputs.back());

    m_initialized = false;
}
