#ifndef POINTER_H
#define POINTER_H

#include <LPointer.h>

using namespace Louvre;

class Pointer final : public LPointer
{
public:
    using LPointer::LPointer;
    void pointerMoveEvent(const LPointerMoveEvent &event) override;
    void pointerButtonEvent(const LPointerButtonEvent &event) override;
};

#endif // POINTER_H
