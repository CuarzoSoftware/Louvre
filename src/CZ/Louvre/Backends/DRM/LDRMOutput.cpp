#include "SRMDevice.h"
#include <CZ/Louvre/Private/LOutputPrivate.h>
#include <CZ/Louvre/Private/LFactory.h>
#include <CZ/Louvre/Backends/DRM/LDRMOutputMode.h>
#include <CZ/Louvre/Backends/DRM/LDRMOutput.h>
#include <CZ/SRM/SRMConnector.h>

using namespace CZ;

static const SRMConnectorInterface ConnIface
{
    .initialized = [](SRMConnector *, void *data)
    {
        auto &output { *static_cast<LDRMOutput*>(data) };
        output.handleInitializeGL();
    },
    .paint = [](SRMConnector *, void *data)
    {
        auto &output { *static_cast<LDRMOutput*>(data) };
        output.handlePaintGL();
    },
    .presented = [](SRMConnector *, const CZPresentationTime &info, void *data)
    {
        auto &output { *static_cast<LDRMOutput*>(data) };
        output.handlePresented(info);
    },
    .discarded = [](SRMConnector *, UInt64 paintEventId, void *data)
    {
        auto &output { *static_cast<LDRMOutput*>(data) };
        output.handleDiscarded(paintEventId);
    },
    .resized = [](SRMConnector *, void *data)
    {
        auto &output { *static_cast<LDRMOutput*>(data) };
        output.handleResizeGL();
    },
    .uninitialized = [](SRMConnector *, void *data)
    {
        auto &output { *static_cast<LDRMOutput*>(data) };
        output.handleUninitializeGL();
    }
};

void LDRMOutput::handleInitializeGL() noexcept
{
    m_output->imp()->backendInitializeGL();
}

void LDRMOutput::handlePaintGL() noexcept
{
    m_output->imp()->backendPaintGL();
}

void LDRMOutput::handleResizeGL() noexcept
{
    m_output->imp()->backendResizeGL();
}

void LDRMOutput::handlePresented(const CZPresentationTime &info) noexcept
{
    m_output->imp()->backendPresented(info);
}

void LDRMOutput::handleDiscarded(UInt64 paintEventId) noexcept
{
    m_output->imp()->backendDiscarded(paintEventId);
}

void LDRMOutput::handleUninitializeGL() noexcept
{
    m_output->imp()->backendUninitializeGL();
}

LOutput *LDRMOutput::Make(SRMConnector *conn) noexcept
{
    LOutput::Params params {};
    params.backend = std::shared_ptr<LDRMOutput>(new LDRMOutput(conn));
    return LFactory::createObject<LOutput>(&params);
}

LDRMOutput::LDRMOutput(SRMConnector *conn) noexcept :
    m_conn(conn)
{
    m_conn->userData = this;
    m_modes.reserve(m_conn->modes().size());

    for (auto *mode : m_conn->modes())
        m_modes.emplace_back(LDRMOutputMode::Make(mode));

    m_desc = std::format("[{}] {} {} {} {}", conn->device()->nodeName(), conn->name(), conn->model(), conn->make(), conn->serial());
}

bool LDRMOutput::init() noexcept
{
    return m_conn->initialize(&ConnIface, this);
}

bool LDRMOutput::repaint() noexcept
{
    return m_conn->repaint();
}

void LDRMOutput::unit() noexcept
{
    m_conn->uninitialize();
}

const std::string &LDRMOutput::name() const noexcept
{
    return m_conn->name();
}

const std::string &LDRMOutput::make() const noexcept
{
    return m_conn->make();
}

const std::string &LDRMOutput::model() const noexcept
{
    return m_conn->model();
}

const std::string &LDRMOutput::desc() const noexcept
{
    return m_desc;
}

const std::string &LDRMOutput::serial() const noexcept
{
    return m_conn->serial();
}

SkISize LDRMOutput::mmSize() const noexcept
{
    return m_conn->mmSize();
}

RSubpixel LDRMOutput::subpixel() const noexcept
{
    return m_conn->subpixel();
}

bool LDRMOutput::isNonDesktop() const noexcept
{
    return m_conn->isNonDesktop();
}

UInt32 LDRMOutput::id() const noexcept
{
    return m_conn->id();
}

RDevice *LDRMOutput::device() const noexcept
{
    return m_conn->device()->reamDevice();
}

void LDRMOutput::setContentType(RContentType type) noexcept
{
    return m_conn->setContentType(type);
}

RContentType LDRMOutput::contentType() const noexcept
{
    return m_conn->contentType();
}

size_t LDRMOutput::gammaSize() const noexcept
{
    return m_conn->gammaSize();
}

std::shared_ptr<const RGammaLUT> LDRMOutput::gammaLUT() const noexcept
{
    return m_conn->gammaLUT();
}

bool LDRMOutput::setGammaLUT(std::shared_ptr<const RGammaLUT> gamma) noexcept
{
    return m_conn->setGammaLUT(gamma);
}

UInt32 LDRMOutput::imageIndex() const noexcept
{
    return m_conn->imageIndex();
}

UInt32 LDRMOutput::imageAge() const noexcept
{
    return m_conn->imageAge();
}

const std::vector<std::shared_ptr<RImage> > &LDRMOutput::images() const noexcept
{
    return m_conn->images();
}

void LDRMOutput::setDamage(const SkRegion &region) noexcept
{
    m_conn->damage = region;
}

bool LDRMOutput::canDisableVSync() const noexcept
{
    return m_conn->canDisableVSync();
}

bool LDRMOutput::isVSyncEnabled() const noexcept
{
    return m_conn->isVSyncEnabled();
}

bool LDRMOutput::enableVSync(bool enabled) noexcept
{
    return m_conn->enableVSync(enabled);
}

UInt64 LDRMOutput::paintEventId() const noexcept
{
    return m_conn->paintEventId();
}

const std::vector<std::shared_ptr<LOutputMode>> &LDRMOutput::modes() const noexcept
{
    return m_modes;
}

const std::shared_ptr<LOutputMode> LDRMOutput::preferredMode() const noexcept
{
    return static_cast<LDRMOutputMode*>(m_conn->preferredMode()->userData)->self();
}

const std::shared_ptr<LOutputMode> LDRMOutput::currentMode() const noexcept
{
    return static_cast<LDRMOutputMode*>(m_conn->currentMode()->userData)->self();
}

int LDRMOutput::setMode(std::shared_ptr<LOutputMode> mode) noexcept
{
    if (auto *srmMode = dynamic_cast<LDRMOutputMode*>(mode.get()))
        return m_conn->setMode(srmMode->srmMode());

    return 0;
}

bool LDRMOutput::hasCursor() const noexcept
{
    return m_conn->hasCursor();
}

bool LDRMOutput::setCursor(UInt8 *pixels) noexcept
{
    return m_conn->setCursor(pixels);
}

bool LDRMOutput::setCursorPos(SkIPoint pos) noexcept
{
    return m_conn->setCursorPos(pos);
}
