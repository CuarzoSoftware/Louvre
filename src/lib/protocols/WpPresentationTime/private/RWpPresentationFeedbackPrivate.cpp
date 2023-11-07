#include <protocols/WpPresentationTime/private/RWpPresentationFeedbackPrivate.h>
#include <protocols/WpPresentationTime/presentation-time.h>

/* Currently has no interface (no requests) */

void RWpPresentationFeedback::RWpPresentationFeedbackPrivate::resource_destroy(wl_resource *resource)
{
    RWpPresentationFeedback *rWpPresentationFeedback = (RWpPresentationFeedback*)wl_resource_get_user_data(resource);
    delete rWpPresentationFeedback;
}
