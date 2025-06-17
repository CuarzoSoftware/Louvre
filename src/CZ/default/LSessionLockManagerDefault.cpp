#include <CZ/Louvre/Events/LTouchCancelEvent.h>
#include <CZ/Louvre/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LClipboard.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/LPointer.h>
#include <CZ/Louvre/LTouch.h>
#include <CZ/Louvre/LDND.h>

using namespace Louvre;

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
        seat()->touch()->sendCancelEvent(LTouchCancelEvent());
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
