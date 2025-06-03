#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

#include <LSurface.h>
#include <LPointer.h>
#include <LBitset.h>

using namespace Louvre;

LPRIVATE_CLASS(LPointer)

    enum StateFlags
    {
        PendingSwipeEndEvent     = static_cast<UInt32>(1) << 0,
        PendingPinchEndEvent     = static_cast<UInt32>(1) << 1,
        PendingHoldEndEvent      = static_cast<UInt32>(1) << 2
    };

    void sendLeaveEvent(LSurface *surface) noexcept;

    LWeak<LSurface> focus;
    LWeak<LSurface> draggingSurface;
    std::vector<LPointerButtonEvent::Button> pressedButtons;
    LBitset<StateFlags> state;
};

#endif // LPOINTERPRIVATE_H
