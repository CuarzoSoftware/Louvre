#ifndef RPRESENTATIONFEEDBACK_H
#define RPRESENTATIONFEEDBACK_H

#include <LResource.h>
#include <LWeak.h>

class Louvre::Protocols::PresentationTime::RPresentationFeedback final : public LResource
{
public:
    LSurface *surface() const noexcept
    {
        return m_surface.get();
    }

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

private:
    friend class Louvre::Protocols::PresentationTime::GPresentation;
    friend class Louvre::Protocols::Wayland::RSurface;
    friend class Louvre::LSurface;
    RPresentationFeedback(GPresentation *presentarionRes,
                          LSurface *surface,
                          UInt32 id) noexcept;

    ~RPresentationFeedback() noexcept;

    LWeak<LSurface> m_surface;
    LWeak<LOutput> m_output;
    Int64 m_commitId { -1 };
    bool m_outputSet { false };
};

#endif // RPRESENTATIONFEEDBACK_H
