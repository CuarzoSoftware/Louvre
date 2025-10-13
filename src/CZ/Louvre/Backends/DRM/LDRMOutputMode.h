#ifndef CZ_LDRMOUTPUTMODE_H
#define CZ_LDRMOUTPUTMODE_H

#include <CZ/Louvre/Seat/LOutputMode.h>
#include <CZ/SRM/SRMConnectorMode.h>
#include <CZ/Core/CZWeak.h>
#include <memory>

namespace CZ
{
class LDRMOutputMode final : public LOutputMode
{
public:
    LOutput *output() const noexcept override;
    static std::shared_ptr<LDRMOutputMode> Make(SRMConnectorMode *mode) noexcept;
    SkISize size() const noexcept override;
    UInt32 refreshRate() const noexcept override;
    bool isPreferred() const noexcept override;

    SRMConnectorMode *srmMode() const noexcept { return m_mode; }
    std::shared_ptr<LDRMOutputMode> self() const noexcept { return m_self.lock(); }
protected:
    LDRMOutputMode(SRMConnectorMode *mode) noexcept;
    CZWeak<SRMConnectorMode> m_mode;
    std::weak_ptr<LDRMOutputMode> m_self;
};
}

#endif // CZ_LDRMOUTPUTMODE_H
