#ifndef LSCENETOUCHPOINTPRIVATE_H
#define LSCENETOUCHPOINTPRIVATE_H

#include <LSceneTouchPoint.h>

using namespace Louvre;

LPRIVATE_CLASS(LSceneTouchPoint)
    Int32 id = 0;
    LPointF pos;
    bool isPressed = true;
    bool listChanged = false;
    std::vector<LView*> views;
    LScene *scene = nullptr;
};

#endif // LSCENETOUCHPOINTPRIVATE_H
