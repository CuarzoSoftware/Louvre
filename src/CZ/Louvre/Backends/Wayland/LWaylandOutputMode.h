#ifndef LWAYLANDOUTPUTMODE_H
#define LWAYLANDOUTPUTMODE_H

#include <CZ/Louvre/Seat/LOutputMode.h>
#include <CZ/SRM/SRMConnectorMode.h>
#include <CZ/Core/CZWeak.h>

namespace CZ
{
class LWaylandOutput;

class LWaylandOutputMode final : public LOutputMode
{
public:
    LWaylandOutputMode(LWaylandOutput *output) noexcept : m_output(output) {}
    LOutput *output() const noexcept override;
    SkISize size() const noexcept override { return m_size; };
    UInt32 refreshRate() const noexcept override { return 60; };
    bool isPreferred() const noexcept override { return true; };
private:
    friend class LWaylandOutput;
    CZWeak<LWaylandOutput> m_output;
    SkISize m_size { 1200, 800 };
};
}

#endif // LWAYLANDOUTPUTMODE_H
