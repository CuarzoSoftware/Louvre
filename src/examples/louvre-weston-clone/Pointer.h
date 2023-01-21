#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Pointer : public LPointer
{
public:
    Pointer(Params *params);
    float lastEventMs;
    void pointerMoveEvent(float dx, float dy) override;

};

#endif // POINTER_H
