#ifndef RPRESENTATIONFEEDBACK_H
#define RPRESENTATIONFEEDBACK_H

#include <CZ/Louvre/LResource.h>
#include <CZ/Core/CZWeak.h>

class CZ::Protocols::PresentationTime::RPresentationFeedback final : public LResource
{
public:
    LSurface *surface() const noexcept { return m_surface; }

    /******************** EVENTS ********************/

    void syncOutput(Wayland::GOutput *outputRes) noexcept;
    void presented(UInt32 tv_sec_hi,
                   UInt32 tv_sec_lo,
                   UInt32 tv_nsec,
                   UInt32 refresh,
                   UInt32 seq_hi,
                   UInt32 seq_lo,
                   UInt32 flags) noexcept;
    void discarded() noexcept;

    CZWeak<LOutput> output;
    std::optional<UInt64> paintEventId;

private:
    friend class CZ::Protocols::PresentationTime::GPresentation;
    friend class CZ::Protocols::Wayland::RWlSurface;
    friend class CZ::LSurface;
    RPresentationFeedback(GPresentation *presentarionRes,
                          LSurface *surface,
                          UInt32 id) noexcept;

    ~RPresentationFeedback() noexcept;

    CZWeak<LSurface> m_surface;
};

#endif // RPRESENTATIONFEEDBACK_H
