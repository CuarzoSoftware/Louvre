#ifndef LPOINTERPRIVATE_H
#define LPOINTERPRIVATE_H

#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Core/CZBitset.h>
#include <CZ/Core/CZWeak.h>

using namespace CZ;

LPRIVATE_CLASS(LPointer)

    enum StateFlags
    {
        PendingSwipeEndEvent     = static_cast<UInt32>(1) << 0,
        PendingPinchEndEvent     = static_cast<UInt32>(1) << 1,
        PendingHoldEndEvent      = static_cast<UInt32>(1) << 2
    };

    void sendLeaveEvent(LSurface *surface) noexcept;

    CZWeak<LSurface> focus, grab;
    CZWeak<LSurface> draggingSurface;
    CZBitset<StateFlags> state;
};

#endif // LPOINTERPRIVATE_H
