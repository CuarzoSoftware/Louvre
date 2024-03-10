#ifndef RPRESENTATIONFEEDBACK_H
#define RPRESENTATIONFEEDBACK_H

#include <LResource.h>

class Louvre::Protocols::PresentationTime::RPresentationFeedback : public LResource
{
public:
    RPresentationFeedback(GPresentation *gPresentation,
                            LSurface *lSurface,
                            UInt32 id);

    ~RPresentationFeedback();

    LSurface *surface() const;

    bool syncOutput(Wayland::GOutput *gOutput) const;
    bool presented(UInt32 tv_sec_hi,
                   UInt32 tv_sec_lo,
                   UInt32 tv_nsec,
                   UInt32 refresh,
                   UInt32 seq_hi,
                   UInt32 seq_lo,
                   UInt32 flags) const;
    bool discarded() const;

    LPRIVATE_IMP_UNIQUE(RPresentationFeedback)
};

#endif // RPRESENTATIONFEEDBACK_H
