#include <protocols/PresentationTime/private/RPresentationFeedbackPrivate.h>
#include <protocols/PresentationTime/private/GPresentationPrivate.h>
#include <protocols/PresentationTime/presentation-time.h>

#include <protocols/Wayland/GOutput.h>

#include <private/LSurfacePrivate.h>

RPresentationFeedback::RPresentationFeedback
(
    GPresentation *gPresentation,
    LSurface *lSurface,
    UInt32 id
)
    :LResource
    (
        gPresentation->client(),
        &wp_presentation_feedback_interface,
        gPresentation->version(),
        id,
        nullptr
    ),
    LPRIVATE_INIT_UNIQUE(RPresentationFeedback)
{
    imp()->surface.reset(lSurface);
    surface()->imp()->presentationFeedbackResources.push_back(this);
}

RPresentationFeedback::~RPresentationFeedback()
{
    if (surface())
        LVectorRemoveOne(surface()->imp()->presentationFeedbackResources, this);
}

Louvre::LSurface *RPresentationFeedback::surface() const
{
    return imp()->surface.get();
}

bool RPresentationFeedback::syncOutput(Wayland::GOutput *gOutput) const
{
    wp_presentation_feedback_send_sync_output(resource(), gOutput->resource());
    return true;
}

bool RPresentationFeedback::presented(UInt32 tv_sec_hi,
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

bool RPresentationFeedback::discarded() const
{
    wp_presentation_feedback_send_discarded(resource());
    return true;
}
