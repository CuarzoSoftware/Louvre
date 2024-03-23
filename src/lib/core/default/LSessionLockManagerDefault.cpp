#include <LTouchCancelEvent.h>
#include <LSessionLockManager.h>
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
    return false;
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
        seat()->pointer()->setFocus(nullptr);
        seat()->keyboard()->setFocus(nullptr);
        seat()->keyboard()->setGrab(nullptr);
        seat()->touch()->sendCancelEvent(LTouchCancelEvent());
        break;
    case DeadLocked:
        break;
    }
}
//! [stateChanged]
