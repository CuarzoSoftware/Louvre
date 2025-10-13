#ifndef LOFFSCREENOUTPUTMODE_H
#define LOFFSCREENOUTPUTMODE_H

#include <CZ/Louvre/Seat/LOutputMode.h>
#include <CZ/SRM/SRMConnectorMode.h>
#include <CZ/Core/CZWeak.h>

namespace CZ
{
class LOffscreenOutput;

class LOffscreenOutputMode : public LOutputMode
{
public:
    LOffscreenOutputMode(LOffscreenOutput *output, SkISize size) noexcept;
    LOutput *output() const noexcept override;
    SkISize size() const noexcept override { return m_size; };
    UInt32 refreshRate() const noexcept override { return m_refreshRate; };
    bool isPreferred() const noexcept override { return true; };

    SkISize m_size;
    UInt32 m_refreshRate { 60 };
private:
    CZWeak<LOffscreenOutput> m_output;
};
}

#endif // LOFFSCREENOUTPUTMODE_H
