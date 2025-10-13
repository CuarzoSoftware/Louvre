#include <CZ/Core/Events/CZTouchCancelEvent.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Seat/LClipboard.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Seat/LDND.h>

using namespace CZ;

//! [lockRequest]
bool LSessionLockManager::lockRequest(LClient *client)
{
    L_UNUSED(client);

    /* Allow all requests by default. */

    return true;
}
//! [lockRequest]

//! [stateChanged]
void LSessionLockManager::stateChanged()
{
    switch (state())
    {
    case Unlocked:
        break;
    case Locked:
        seat()->dnd()->cancel();
        seat()->touch()->sendCancelEvent({});
        seat()->keyboard()->setGrab(nullptr);
        seat()->pointer()->setDraggingSurface(nullptr);

        if (cursor()->output() && cursor()->output()->sessionLockRole())
        {
            seat()->pointer()->setFocus(cursor()->output()->sessionLockRole()->surface());
            seat()->keyboard()->setFocus(cursor()->output()->sessionLockRole()->surface());
        }
        else
        {
            seat()->pointer()->setFocus(nullptr);
            seat()->keyboard()->setFocus(nullptr);
        }
        break;
    case DeadLocked:
        break;
    }
}
//! [stateChanged]
