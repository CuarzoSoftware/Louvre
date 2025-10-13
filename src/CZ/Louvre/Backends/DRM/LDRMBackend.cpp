#include <CZ/Louvre/Backends/DRM/LDRMBackend.h>
#include <CZ/Louvre/Backends/DRM/LDRMOutput.h>
#include <CZ/Louvre/Private/LSeatPrivate.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LDMAFeedback.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Ream/RCore.h>
#include <CZ/SRM/SRMCore.h>
#include <CZ/SRM/SRMDevice.h>
#include <CZ/SRM/SRMConnector.h>
#include <CZ/SRM/SRMEncoder.h>
#include <CZ/SRM/SRMPlane.h>
#include <CZ/SRM/SRMCrtc.h>
#include <CZ/SRM/SRMLease.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ;

LDRMBackend::LDRMBackend() noexcept : LBackend(LBackendId::DRM)
{
    iLog = LLog.newWithContext("Libinput Backend");
    gLog = LLog.newWithContext("DRM Backend");
}

static const SRMInterface srmIface
{
    .openRestricted = [](const char *path, int flags, void *data)
    {
        auto &backend { *static_cast<LDRMBackend*>(data) };
        return backend.openDRMDevice(path, flags);
    },

    .closeRestricted = [](int fd, void *data)
    {
        auto &backend { *static_cast<LDRMBackend*>(data) };
        backend.closeDRMDevice(fd);
    }
};

int LDRMBackend::openDRMDevice(const char *path, int flags) noexcept
{
    if (m_libseatEnabled)
    {
        SeatDevice dev {};
        dev.id = seat()->openDevice(path, &dev.fd);

        if (dev.id < 0)
            return -1;

        m_drmSeatDevices.emplace_back(dev);
        return dev.fd;
    }
    else
        return open(path, flags);
}

void LDRMBackend::closeDRMDevice(int fd) noexcept
{
    if (m_libseatEnabled)
    {
        SeatDevice dev {};

        for (size_t i = 0; i < m_drmSeatDevices.size(); i++)
        {
            if (m_drmSeatDevices[i].fd == fd)
            {
                dev = m_drmSeatDevices[i];
                m_drmSeatDevices[i] = m_drmSeatDevices.back();
                m_drmSeatDevices.pop_back();
                break;
            }
        }

        if (dev.fd == -1)
            return;

        seat()->closeDevice(dev.id);
    }
    else
        close(fd);
}

void LDRMBackend::initPresentationClock() noexcept
{
    // Guaranteed to be at least 1 device
    // Note: Starting kernel version 4.15, this is always MONOTONIC
    for (auto *dev : m_srm->devices())
    {
        m_presentationClock = dev->presentationClock();
        return;
    }
}

void LDRMBackend::initDefaultFeedback() noexcept
{
    auto ream { RCore::Get() };
    auto supportedFormats { RDRMFormatSet::Intersect(ream->mainDevice()->dmaTextureFormats(), ream->mainDevice()->textureFormats()) };

    if (ream->mainDevice()->id() == 0 || supportedFormats.formats().empty())
        return;

    std::vector<LDMAFeedback::Tranche> tranches;
    tranches.reserve(4);

    // Main device texture tranche
    LDMAFeedback::Tranche tranche {};
    tranche.device = ream->mainDevice();
    tranche.flags = 0;
    tranche.formatSet = supportedFormats;
    tranches.emplace_back(std::move(tranche));

    for (auto *dev : m_srm->devices())
    {
        if (!dev->reamDevice() || dev->reamDevice()->id() == 0)
            continue;

        // Other device texture tranche
        if (dev->reamDevice() != ream->mainDevice())
        {
            auto set { RDRMFormatSet::Intersect(supportedFormats, dev->reamDevice()->dmaTextureFormats()) };
            set.removeModifier(DRM_FORMAT_MOD_INVALID);

            if (!set.formats().empty())
            {
                tranche.device = dev->reamDevice();
                tranche.flags = 0;
                tranche.formatSet = std::move(set);
                tranches.emplace_back(std::move(tranche));
            }
        }

        RDRMFormatSet scanout { supportedFormats };

        for (auto *plane : dev->planes())
        {
            if (plane->type() != SRMPlane::Primary)
                continue;

            scanout = RDRMFormatSet::Intersect(scanout, plane->formats());
        }

        if (dev->reamDevice() != ream->mainDevice())
            scanout.removeModifier(DRM_FORMAT_MOD_INVALID);

        if (!scanout.formats().empty())
        {
            tranche.device = dev->reamDevice();
            tranche.flags = LDMAFeedback::Scanout;
            tranche.formatSet = std::move(scanout);
            tranches.emplace_back(std::move(tranche));
        }
    }

    m_defaultFeedback = LDMAFeedback::Make(ream->mainDevice(), std::move(tranches));
}

bool LDRMBackend::init() noexcept
{
    m_libseatEnabled = seat()->imp()->initLibseat();
    m_udev = udev_new();

    if (!m_udev)
        return false;

    if (!inputInit())
        return false;

    m_srm = SRMCore::Make(&srmIface, this);

    if (!m_srm)
        return false;

    initPresentationClock();
    initDefaultFeedback();

    m_srm->onConnectorPlugged.subscribe(this, [this](SRMConnector *conn) {
        m_outputs.emplace_back(LDRMOutput::Make(conn));
        seat()->imp()->handleOutputPlugged(m_outputs.back());
    });

    m_srm->onConnectorUnplugged.subscribe(this, [this](SRMConnector *conn) {
        auto *output { static_cast<LDRMOutput*>(conn->userData)->output() };
        seat()->imp()->handleOutputUnplugged(output);
        CZVectorUtils::RemoveOneUnordered(m_outputs, output);

        // TODO: Prevent users from calling initialize() again...
        compositor()->onAnticipatedObjectDestruction(output);
        delete output;
    });

    for (auto *dev : m_srm->devices())
        for (auto *conn : dev->connectors())
            if (conn->isConnected())
                m_outputs.emplace_back(LDRMOutput::Make(conn));

    return true;
}

void LDRMBackend::unit() noexcept
{
    if (m_srm)
        m_srm->suspend();

    m_defaultFeedback.reset();
    m_srm.reset();

    assert(m_outputs.empty());

    m_libinputEventSource.reset();

    if (m_libinput)
    {
        libinput_unref(m_libinput);
        m_libinput = {};
    }

    if (m_udev)
    {
        udev_unref(m_udev);
        m_udev = {};
    }
}

void LDRMBackend::suspend() noexcept
{
    m_srm->suspend();
    inputSuspend();
}

void LDRMBackend::resume() noexcept
{
    inputResume();
    m_srm->resume();
}

const std::vector<LOutput *> &LDRMBackend::outputs() const noexcept
{
    return m_outputs;
}

std::shared_ptr<LDMAFeedback> LDRMBackend::defaultFeedback() const noexcept
{
    return m_defaultFeedback;
}

static bool AddConnRes(SRMLease::Resources &res, SRMConnector *conn) noexcept
{
    int bestScore { 0 };
    SRMDevice *dev { conn->device() };
    SRMEncoder *bestEncoder {};
    SRMCrtc *bestCrtc {};
    SRMPlane *bestPrimaryPlane {};
    SRMPlane *bestCursorPlane {};

    for (auto *encoder : conn->encoders())
    {
        for (auto *crtc : encoder->crtcs())
        {
            if (res.crtcs.contains(crtc) || crtc->currentConnector() || crtc->leased())
                continue;

            int score { 0 };
            SRMPlane *primaryPlane { nullptr };
            SRMPlane *cursorPlane { nullptr };

            for (auto *plane : dev->planes())
            {
                if (plane->type() == SRMPlane::Overlay || plane->leased() || plane->currentConnector() || res.planes.contains(plane))
                    continue;

                for (auto *planeCrtc : plane->crtcs())
                {
                    if (planeCrtc->id() == crtc->id())
                    {
                        if (plane->type() == SRMPlane::Primary)
                        {
                            primaryPlane = plane;
                            break;
                        }
                        else if (plane->type() == SRMPlane::Cursor && !plane->device()->core()->forcingLegacyCursor())
                        {
                            cursorPlane = plane;
                            break;
                        }
                    }
                }
            }

            if (!primaryPlane)
                continue;

            score += 100;

            if (score > bestScore)
            {
                bestScore = score;
                bestEncoder = encoder;
                bestCrtc = crtc;
                bestPrimaryPlane = primaryPlane;
                bestCursorPlane = cursorPlane;
            }
        }
    }

    if (bestEncoder && bestCrtc && bestPrimaryPlane)
    {
        res.connectors.emplace(conn);
        res.crtcs.emplace(bestCrtc);
        res.planes.emplace(bestPrimaryPlane);

        if (bestCursorPlane)
            res.planes.emplace(bestCursorPlane);

        return true;
    }

    return false;
}

std::shared_ptr<SRMLease> LDRMBackend::createLease(const std::unordered_set<LOutput *> &outputs) noexcept
{
    assert(!outputs.empty());
    SRMLease::Resources res {};

    auto *dev { (*outputs.begin())->device()->srmDevice() };

    for (auto *output : outputs)
    {
        auto *conn { static_cast<LDRMOutput*>(output->backend())->conn() };

        if (!AddConnRes(res, conn))
        {
            gLog(CZWarning, CZLN, "Failed to find resources for connector {}", conn->id());
            continue;
        }
    }

    return dev->createLease(std::move(res));
}

