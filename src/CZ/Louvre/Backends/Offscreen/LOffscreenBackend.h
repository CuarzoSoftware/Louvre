#ifndef LOFFSCREENBACKEND_H
#define LOFFSCREENBACKEND_H

#include <CZ/Louvre/Backends/LBackend.h>

class CZ::LOffscreenBackend : public LBackend
{
public:
    LOffscreenBackend() noexcept : LBackend(LBackendId::Offscreen) {};

    LOutput *addOutput(SkISize modeSize) noexcept;
    void removeOutput(LOutput *output) noexcept;

    /* Common */

    bool init() noexcept override;
    void unit() noexcept override;
    void suspend() noexcept override {};
    void resume() noexcept override {};

    /* Input */

    const std::set<std::shared_ptr<CZInputDevice>> &inputDevices() const noexcept override { return m_inputDevices; };
    void inputSetLeds(UInt32 /*leds*/) noexcept override {};
    void inputForceUpdate() noexcept override {};

    /* Output */

    const std::vector<LOutput*> &outputs() const noexcept override { return m_outputs; };
    std::shared_ptr<LDMAFeedback> defaultFeedback() const noexcept override { return m_defaultFeedback; };
    clockid_t presentationClock() const noexcept override { return CLOCK_MONOTONIC; };
    std::shared_ptr<SRMLease> createLease(const std::unordered_set<LOutput*> &) noexcept override { return {}; };

    std::shared_ptr<RCore> m_ream;
    std::vector<LOutput*> m_outputs;
    std::set<std::shared_ptr<CZInputDevice>> m_inputDevices;
    std::shared_ptr<LDMAFeedback> m_defaultFeedback;
    bool m_initialized { false };
};

#endif // LOFFSCREENBACKEND_H
