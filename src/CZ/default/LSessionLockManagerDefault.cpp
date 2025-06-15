#include <LTouchCancelEvent.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <LOutput.h>
#include <LCursor.h>
#include <LSeat.h>
#include <LClipboard.h>
#include <LKeyboard.h>
#include <LPointer.h>
#include <LTouch.h>
#include <LDND.h>

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
