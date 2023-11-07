#ifndef RWPPRESENTATIONFEEDBACK_H
#define RWPPRESENTATIONFEEDBACK_H

#include <LResource.h>

class Louvre::Protocols::WpPresentationTime::RWpPresentationFeedback : public LResource
{
public:
    RWpPresentationFeedback(GWpPresentation *gWpPresentation,
                            LSurface *lSurface,
                            UInt32 id);

    ~RWpPresentationFeedback();

    LSurface *lSurface() const;

    bool sync_output(Wayland::GOutput *gOutput) const;
    bool presented(UInt32 tv_sec_hi,
                   UInt32 tv_sec_lo,
                   UInt32 tv_nsec,
                   UInt32 refresh,
                   UInt32 seq_hi,
                   UInt32 seq_lo,
                   UInt32 flags) const;
    bool discarded() const;

    LPRIVATE_IMP(RWpPresentationFeedback)
};

#endif // RWPPRESENTATIONFEEDBACK_H
