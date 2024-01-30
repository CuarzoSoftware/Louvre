#include <protocols/WpPresentationTime/private/RWpPresentationFeedbackPrivate.h>
#include <protocols/WpPresentationTime/private/GWpPresentationPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>

#include <protocols/Wayland/GOutput.h>

#include <private/LSurfacePrivate.h>

RWpPresentationFeedback::RWpPresentationFeedback
(
    GWpPresentation *gWpPresentation,
    LSurface *lSurface,
    UInt32 id
)
    :LResource
    (
        gWpPresentation->client(),
        &wp_presentation_feedback_interface,
        gWpPresentation->version(),
        id,
        nullptr,
        &RWpPresentationFeedbackPrivate::resource_destroy
    ),
    LPRIVATE_INIT_UNIQUE(RWpPresentationFeedback)
{
    imp()->lSurface = lSurface;
    this->lSurface()->imp()->wpPresentationFeedbackResources.push_back(this);
}

RWpPresentationFeedback::~RWpPresentationFeedback()
{
    if (lSurface())
        LVectorRemoveOne(lSurface()->imp()->wpPresentationFeedbackResources, this);
}

Louvre::LSurface *RWpPresentationFeedback::lSurface() const
{
    return imp()->lSurface;
}

bool RWpPresentationFeedback::sync_output(Wayland::GOutput *gOutput) const
{
    wp_presentation_feedback_send_sync_output(resource(), gOutput->resource());
    return true;
}

bool RWpPresentationFeedback::presented(UInt32 tv_sec_hi,
                                        UInt32 tv_sec_lo,
                                        UInt32 tv_nsec,
                                        UInt32 refresh,
                                        UInt32 seq_hi,
                                        UInt32 seq_lo,
                                        UInt32 flags) const
{
    wp_presentation_feedback_send_presented(resource(),
                                            tv_sec_hi,
                                            tv_sec_lo,
                                            tv_nsec,
                                            refresh,
                                            seq_hi,
                                            seq_lo,
                                            flags);
    return true;
}

bool RWpPresentationFeedback::discarded() const
{
    wp_presentation_feedback_send_discarded(resource());
    return true;
}
