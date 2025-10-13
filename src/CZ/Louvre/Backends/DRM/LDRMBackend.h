#ifndef LDRMBACKEND_H
#define LDRMBACKEND_H

#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/SRM/SRM.h>
#include <CZ/Core/CZEventSource.h>
#include <libinput.h>

class CZ::LDRMBackend : public LBackend
{
public:
    struct SeatDevice
    {
        int fd { -1 };
        int id { -1 };
    };

    LDRMBackend() noexcept;

    bool init() noexcept override;
    void unit() noexcept override;
    void suspend() noexcept override;
    void resume() noexcept override;

    /* Graphics */

    int openDRMDevice(const char *path, int flags) noexcept;
    void closeDRMDevice(int fd) noexcept;
    void initPresentationClock() noexcept;
    void initDefaultFeedback() noexcept;

    const std::vector<LOutput*> &outputs() const noexcept override;    
    std::shared_ptr<LDMAFeedback> defaultFeedback() const noexcept override;
    clockid_t presentationClock() const noexcept override { return m_presentationClock; };
    std::shared_ptr<SRMLease> createLease(const std::unordered_set<LOutput*> &outputs) noexcept override;

    /* Input */

    bool inputInit() noexcept;
    void inputSuspend() noexcept;
    void inputResume() noexcept;
    void inputDispatch() noexcept;
    const std::set<std::shared_ptr<CZInputDevice> > &inputDevices() const noexcept override;
    void inputSetLeds(UInt32 leds) noexcept override;
    void inputForceUpdate() noexcept override;
    int  inputOpenDevice(const char *path, int flags) noexcept;
    void inputCloseDevice(int fd) noexcept;

protected:
    udev *m_udev { nullptr };
    bool m_libseatEnabled { false };
    CZLogger gLog, iLog;

    std::shared_ptr<SRMCore> m_srm;
    std::vector<LOutput*> m_outputs;
    std::shared_ptr<LDMAFeedback> m_defaultFeedback;
    std::vector<SeatDevice> m_drmSeatDevices;

    libinput *m_libinput { nullptr };
    std::shared_ptr<CZEventSource> m_libinputEventSource;
    std::set<std::shared_ptr<CZInputDevice>> m_inputDevices;
    std::vector<SeatDevice> m_inputSeatDevices;
    clockid_t m_presentationClock;
};

#endif // LDRMBACKEND_H
