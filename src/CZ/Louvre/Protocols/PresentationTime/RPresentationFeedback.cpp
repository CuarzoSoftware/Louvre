#include <CZ/Louvre/Protocols/PresentationTime/presentation-time.h>
#include <CZ/Louvre/Protocols/PresentationTime/RPresentationFeedback.h>
#include <CZ/Louvre/Protocols/PresentationTime/GPresentation.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Private/LSurfacePrivate.h>
#include <CZ/Core/Utils/CZVectorUtils.h>

using namespace CZ::Protocols::PresentationTime;

RPresentationFeedback::RPresentationFeedback
(
    GPresentation *presentationRes,
    LSurface *surface,
    UInt32 id
) noexcept
    :LResource
    (
        presentationRes->client(),
        &wp_presentation_feedback_interface,
        presentationRes->version(),
        id,
        nullptr
    ),
    m_surface(surface)
{
    surface->imp()->pending.presentationFeedbackRes.emplace_back(this);
}

RPresentationFeedback::~RPresentationFeedback() noexcept {}

void RPresentationFeedback::syncOutput(Wayland::GOutput *outputRes) noexcept
{
    wp_presentation_feedback_send_sync_output(resource(), outputRes->resource());
}

void RPresentationFeedback::presented(UInt32 tv_sec_hi,
                                        UInt32 tv_sec_lo,
                                        UInt32 tv_nsec,
                                        UInt32 refresh,
                                        UInt32 seq_hi,
                                        UInt32 seq_lo,
                                        UInt32 flags) noexcept
{
    wp_presentation_feedback_send_presented(resource(),
                                            tv_sec_hi,
                                            tv_sec_lo,
                                            tv_nsec,
                                            refresh,
                                            seq_hi,
                                            seq_lo,
                                            flags);
    destroy();
}

void RPresentationFeedback::discarded() noexcept
{
    wp_presentation_feedback_send_discarded(resource());
    destroy();
}
