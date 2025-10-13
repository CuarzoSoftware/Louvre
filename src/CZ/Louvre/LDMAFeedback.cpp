#include <CZ/Louvre/LDMAFeedback.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;

#pragma pack(push, 1)
struct FMT
{
    RFormat fmt;
    UInt32 pad;
    RModifier mod;
};
#pragma pack(pop)

std::shared_ptr<LDMAFeedback> LDMAFeedback::Make(RDevice *mainDevice, std::vector<Tranche> &&tranches) noexcept
{
    if (!mainDevice)
    {
        LLog(CZError, CZLN, "Missing main device");
        return {};
    }

    bool mainDeviceHasTranche { false };
    size_t totalFormats { 0 };

    for (auto &tranche : tranches)
    {
        if (!tranche.device)
        {
            LLog(CZError, CZLN, "Missing tranche device");
            return {};
        }

        if (tranche.formatSet.formats().empty())
        {
            LLog(CZError, CZLN, "The format set is empty");
            return {};
        }

        mainDeviceHasTranche |= tranche.device == mainDevice;

        size_t formats { 0 };
        for (auto &fmt : tranche.formatSet.formats())
            formats += fmt.modifiers().size();

        tranche.m_fmtIdx.resize(formats);
        totalFormats += formats;
    }

    // TODO: Validate that formats are not repeated across tranches with the same device and flags

    if (!mainDeviceHasTranche)
    {
        LLog(CZError, CZLN, "No tranche found for the main device");
        return {};
    }

    auto shm { CZSharedMemory::Make(totalFormats * sizeof(FMT), O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC, 0666) };

    if (!shm)
    {
        LLog(CZError, CZLN, "Failed to alloc SHM for format table");
        return {};
    }

    // Global table index
    UInt16 tableIndex {};
    auto *table { (FMT*)shm->map() };

    for (auto &tranche : tranches)
    {
        // Format array index
        size_t formatIndex {};

        for (auto &fmt : tranche.formatSet.formats())
        {
            for (auto mod : fmt.modifiers())
            {
                tranche.m_fmtIdx[formatIndex++] = tableIndex;
                table->fmt = fmt.format();
                table->mod = mod;

                tableIndex++;
                table++;
            }
        }
    }

    return std::shared_ptr<LDMAFeedback>(new LDMAFeedback(mainDevice, std::move(tranches), std::move(shm)));
}
