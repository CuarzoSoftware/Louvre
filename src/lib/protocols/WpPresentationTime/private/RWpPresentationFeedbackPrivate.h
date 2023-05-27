#ifndef RWPPRESENTATIONFEEDBACKPRIVATE_H
#define RWPPRESENTATIONFEEDBACKPRIVATE_H

#include <protocols/WpPresentationTime/RWpPresentationFeedback.h>

using namespace Louvre::Protocols::WpPresentationTime;
using namespace std;

LPRIVATE_CLASS(RWpPresentationFeedback)
    static void resource_destroy(wl_resource *resource);
    LSurface *lSurface = nullptr;
    list<RWpPresentationFeedback*>::iterator surfaceLink;
};

#endif // RWPPRESENTATIONFEEDBACKPRIVATE_H
