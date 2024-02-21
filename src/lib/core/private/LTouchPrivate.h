#ifndef LTOUCHPRIVATE_H
#define LTOUCHPRIVATE_H

#include <LTouch.h>

using namespace Louvre;

struct LTouch::Params
{
    /* Add here any required constructor param */
};

LPRIVATE_CLASS(LTouch)
    std::list<LTouchPoint*> touchPoints;
};

#endif // LTOUCHPRIVATE_H
