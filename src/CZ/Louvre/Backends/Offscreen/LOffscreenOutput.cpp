#include <CZ/Louvre/Backends/Offscreen/LOffscreenOutputMode.h>
#include <CZ/Louvre/Backends/Offscreen/LOffscreenOutput.h>
#include <CZ/Louvre/Backends/Offscreen/LOffscreenBackend.h>
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Ream/RCore.h>

using namespace CZ;

LOffscreenOutput::LOffscreenOutput(LOffscreenBackend *backend, SkISize modeSize) noexcept : backend(backend)
{
    info.id = backend->outputs().size() + 1;
    info.name = std::format("Offscreen-{}", info.id);
    info.make = "Cuarzo Software";
    info.model = "Raster";
    info.desc = "Offscreen Output";
    info.serial = std::format("{}", info.id);
    info.age = 0;
    info.images[0] = RImage::Make(modeSize, { DRM_FORMAT_ARGB8888, { DRM_FORMAT_MOD_LINEAR } });
    assert(info.images[0]);
    info.modes.emplace_back(std::shared_ptr<LOffscreenOutputMode>(new LOffscreenOutputMode(this, modeSize)));
}

LOffscreenOutput::~LOffscreenOutput() noexcept
{
    LLog(CZInfo, CZLN, "Offscreen output destroyed");
}

bool LOffscreenOutput::init() noexcept
{
    unitPromise.reset();
    info.age = 0;

    std::thread([this]{

        output()->imp()->backendInitializeGL();

        info.time = {};
        info.time.period = 1000000000 / 60;

        while (true)
        {
            semaphore.acquire();

            if (unitPromise.has_value())
                break;

            output()->imp()->backendPaintGL();

            // Fake 60 Hz
            usleep(1000000/60);
            clock_gettime(CLOCK_MONOTONIC, &info.time.time);
            output()->imp()->backendPresented(info.time);

            info.time.seq++;
            info.time.paintEventId++;
            info.age = 1;
        }

        output()->imp()->backendUninitializeGL();

    }).detach();

    return true;
}

bool LOffscreenOutput::repaint() noexcept
{
    semaphore.release();
    return true;
}

void LOffscreenOutput::unit() noexcept
{
    unitPromise = std::promise<bool>();
    auto unitFuture { unitPromise->get_future() };
    semaphore.release();
}

RDevice *LOffscreenOutput::device() const noexcept
{
    backend->m_ream->mainDevice();
}
