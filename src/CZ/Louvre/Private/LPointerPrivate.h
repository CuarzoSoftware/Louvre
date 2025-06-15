#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

#include <LSurface.h>
#include <LPointer.h>
#include <CZ/CZBitset.h>

using namespace Louvre;

LPRIVATE_CLASS(LPointer)

    enum StateFlags
    {
        PendingSwipeEndEvent     = static_cast<UInt32>(1) << 0,
        PendingPinchEndEvent     = static_cast<UInt32>(1) << 1,
        PendingHoldEndEvent      = static_cast<UInt32>(1) << 2
    };

    void sendLeaveEvent(LSurface *surface) noexcept;

    CZWeak<LSurface> focus;
    CZWeak<LSurface> draggingSurface;
    std::vector<LPointerButtonEvent::Button> pressedButtons;
    CZBitset<StateFlags> state;
};

#endif // LPOINTERPRIVATE_H
