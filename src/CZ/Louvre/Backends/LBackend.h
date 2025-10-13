#ifndef CZ_LBACKEND_H
#define CZ_LBACKEND_H

#include <CZ/SRM/SRM.h>
#include <CZ/Ream/RImage.h>
#include <CZ/Louvre/LObject.h>
#include <memory>
#include <set>

class CZ::LBackend : public LObject
{
public:
    LBackendId id() const noexcept { return m_id; }

    /* Common */

    virtual bool init() noexcept = 0;
    virtual void unit() noexcept = 0;
    virtual void suspend() noexcept = 0;
    virtual void resume() noexcept = 0;

    /* Input */

    virtual const std::set<std::shared_ptr<CZInputDevice>> &inputDevices() const noexcept = 0;
    virtual void inputSetLeds(UInt32 leds) noexcept = 0;
    virtual void inputForceUpdate() noexcept = 0;

    /* Output */

    virtual const std::vector<LOutput*> &outputs() const noexcept = 0;
    virtual std::shared_ptr<LDMAFeedback> defaultFeedback() const noexcept = 0;
    virtual clockid_t presentationClock() const noexcept = 0;
    virtual std::shared_ptr<SRMLease> createLease(const std::unordered_set<LOutput*> &outputs) noexcept = 0;

protected:
    LBackend(LBackendId id) noexcept :
        m_id(id) {};
    LBackendId m_id;
};

#endif // CZ_LBACKEND_H
