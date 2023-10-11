#include <LKeyboard.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LClient.h>
#include <LCursor.h>
#include <LOutput.h>
#include <unistd.h>

using namespace Louvre;

//! [keyModifiersEvent]
void LKeyboard::keyModifiersEvent(UInt32 depressed, UInt32 latched, UInt32 locked, UInt32 group)
{
    sendModifiersEvent(depressed, latched, locked, group);
}
//! [keyModifiersEvent]

//! [keyEvent]
void LKeyboard::keyEvent(UInt32 keyCode, KeyState keyState)
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
                exit(system("weston-terminal"));
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

        // Screenshot
        else if (L_CTRL && L_SHIFT && keyCode == KEY_3)
        {
            if (cursor()->output()->bufferTexture(0))
            {
                const char *user = getenv("HOME");

                if (!user)
                    return;

                char path[128];
                char timeString[32];

                time_t currentTime;
                struct tm *timeInfo;

                time(&currentTime);
                timeInfo = localtime(&currentTime);
                strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeInfo);

                sprintf(path, "%s/Desktop/Louvre_Screenshoot_%s.png", user, timeString);

                cursor()->output()->bufferTexture(0)->save(path);
            }
        }

        // Terminates the compositor
        else if (keyCode == KEY_ESC && L_CTRL && L_SHIFT)
        {
            compositor()->finish();
            return;
        }
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
