#include <LKeyboard.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LClient.h>
#include <unistd.h>

using namespace Louvre;

//! [keyModifiersEvent]
void LKeyboard::keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    sendModifiersEvent(depressed, latched, locked, group);
}
//! [keyModifiersEvent]

//! [keyEvent]
void LKeyboard::keyEvent(UInt32 keyCode, UInt32 keyState)
{
    sendKeyEvent(keyCode, keyState);

    bool L_CTRL = isKeyCodePressed(KEY_LEFTCTRL);
    bool L_SHIFT = isKeyCodePressed(KEY_LEFTSHIFT);
    bool mods = isKeyCodePressed(KEY_LEFTALT) && L_CTRL;

    if (keyState == Released)
    {
        // Launches weston-terminal
        if (keyCode == KEY_F1 && !mods)
        {
            if (fork() == 0)
            {
                execl("/usr/bin/weston-terminal", "weston-terminal", NULL);
                exit(0);
            }
        }

        // Terminates client connection
        else if (L_CTRL && keySymbol(keyCode) == XKB_KEY_q)
        {
            if (focusSurface())
                focusSurface()->client()->destroy();
        }

        // Minimizes currently focused surface
        else if (L_CTRL && keySymbol(keyCode) == XKB_KEY_m)
        {
            if (focusSurface())
                focusSurface()->setMinimized(true);
        }

        // Terminates the compositor
        else if (keyCode == KEY_ESC && L_CTRL && L_SHIFT)
            compositor()->finish();

        else if (L_CTRL && !L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);
        else if (!L_CTRL && L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
        else if (!L_CTRL && !L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::NoAction);
    }

    // Key press
    else
    {
        // CTRL sets Copy as the preferred action in drag & drop sesión
        if (L_CTRL)
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);

        // SHIFT sets the Move as the preferred action in drag & drop sesión
        else if (L_SHIFT)
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
    }
}
//! [keyEvent]

//! [focusChanged]
void LKeyboard::focusChanged()
{
    /* No default implementation. */
}
//! [focusChanged]
