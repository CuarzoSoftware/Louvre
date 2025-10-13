#ifndef CZ_LDMAFEEDBACK_H
#define CZ_LDMAFEEDBACK_H

#include <CZ/Louvre/LObject.h>
#include <CZ/Ream/RDevice.h>
#include <CZ/Core/CZSharedMemory.h>
#include <CZ/Core/CZBitset.h>

class CZ::LDMAFeedback : public LObject
{
public:
    enum TrancheFlag
    {
        Scanout = 1
    };

    struct Tranche
    {
        RDevice *device;
        RDRMFormatSet formatSet;
        CZBitset<TrancheFlag> flags;

        const std::vector<UInt16> &formatIdx() const noexcept { return m_fmtIdx; }
    private:
        friend class LDMAFeedback;
        std::vector<UInt16> m_fmtIdx;
    };

    static std::shared_ptr<LDMAFeedback> Make(RDevice *mainDevice, std::vector<Tranche> &&tranches) noexcept;
    RDevice *mainDevice() const noexcept { return m_mainDevice; }
    const std::vector<Tranche> &tranches() const noexcept { return m_tranches; }
    std::shared_ptr<CZSharedMemory> table() const noexcept { return m_shm; }
protected:
    LDMAFeedback(RDevice *mainDevice, std::vector<Tranche> &&tranches, std::shared_ptr<CZSharedMemory> shm) noexcept :
        m_mainDevice(mainDevice), m_tranches(std::move(tranches)), m_shm(shm) {}
    RDevice *m_mainDevice;
    std::vector<Tranche> m_tranches;
    std::shared_ptr<CZSharedMemory> m_shm;
};

#endif // CZ_LDMAFEEDBACK_H
