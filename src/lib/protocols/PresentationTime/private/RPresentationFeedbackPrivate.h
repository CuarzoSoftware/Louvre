#ifndef RPRESENTATIONFEEDBACKPRIVATE_H
#define RPRESENTATIONFEEDBACKPRIVATE_H

#include <protocols/PresentationTime/RPresentationFeedback.h>
#include <LSurface.h>
#include <LOutput.h>

using namespace Louvre::Protocols::PresentationTime;
using namespace std;

LPRIVATE_CLASS(RPresentationFeedback)
    LWeak<LSurface> surface;
    LWeak<LOutput> output;
    Int64 commitId { -1 };
    bool outputSet { false };
};

#endif // RPRESENTATIONFEEDBACKPRIVATE_H
