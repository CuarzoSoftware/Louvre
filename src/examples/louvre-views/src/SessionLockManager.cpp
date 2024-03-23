#include "SessionLockManager.h"
#include "Compositor.h"
#include "Global.h"
#include <LCursor.h>
#include <LOutput.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LSessionLockRole.h>
#include <LTouchCancelEvent.h>
#include <LDND.h>
#include <LSceneView.h>

void SessionLockManager::stateChanged()
{
    const bool locked { state() != Unlocked };

    G::compositor()->backgroundLayer.setVisible(!locked);
    G::compositor()->surfacesLayer.setVisible(!locked);
    G::compositor()->workspacesLayer.setVisible(!locked);
    G::compositor()->fullscreenLayer.setVisible(!locked);

    if (locked)
    {
        if (cursor()->output() && cursor()->output()->sessionLockRole())
        {
            seat()->keyboard()->setGrab(nullptr);
            seat()->keyboard()->setFocus(cursor()->output()->sessionLockRole()->surface());
            seat()->pointer()->setFocus(cursor()->output()->sessionLockRole()->surface());
        }

        seat()->touch()->sendCancelEvent(LTouchCancelEvent());
        seat()->dnd()->cancel();
    }

    compositor()->repaintAllOutputs();
}
