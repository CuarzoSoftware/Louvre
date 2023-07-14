#include "LOutputMode.h"
#include <private/LClientPrivate.h>
#include <private/LDataDevicePrivate.h>

#include <LKeyboard.h>
#include <LTime.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LClient.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LLog.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

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

    if (keyState == LIBINPUT_KEY_STATE_RELEASED)
    {
        // F1: Launches weston-terminal.
        if (keyCode == KEY_F1 && !mods)
        {
            if (fork() == 0)
            {
                execl("/usr/bin/weston-terminal", "weston-terminal", NULL);
                exit(0);
            }
        }

        else if (L_SHIFT + L_CTRL && keySymbol(keyCode) == XKB_KEY_Down)
        {
            int i = 0;
            for (LOutputMode *m : cursor()->output()->modes())
            {
                LLog::debug("Mode %d (%d, %d)", i, m->sizeB().w(), m->sizeB().h());
                if (i == 19)
                {
                    cursor()->output()->setMode(m);
                    break;
                }
                i++;
            }
        }

        else if (L_CTRL && keySymbol(keyCode) == XKB_KEY_q)
        {
            if (focusSurface())
            {
                wl_client_destroy(focusSurface()->client()->client());
            }
        }

        else if (L_CTRL && keySymbol(keyCode) == XKB_KEY_m)
        {
            if (focusSurface())
                focusSurface()->setMinimized(true);
        }

        // CTRL + SHIFT + ESC : Kills the compositor.
        else if (keyCode == KEY_ESC && L_CTRL && L_SHIFT)
        {
            LLog::warning("Killing compositor.");
            compositor()->finish();
        }

        // F8 : Unminimize all surfaces.
        else if (keyCode == KEY_F8 && !mods)
        {
            std::list<LSurface*>surfaces = compositor()->surfaces();
            for (LSurface *surface : surfaces)
                surface->setMinimized(false);

            compositor()->repaintAllOutputs();
        }

        // CTRL: Unsets the **Copy** as the preferred action in drag & drop sesión
        // SHIFT: Unsets the **Move** as the preferred action in drag & drop sesión
        else if (L_CTRL || L_SHIFT)
        {
            seat()->dndManager()->setPreferredAction(LDNDManager::NoAction);
        }
    }
    // Key press
    else
    {
        // CTRL: Sets the **Copy** as the preferred action in drag & drop sesión
        if (L_CTRL)
        {
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);
        }

        // SHIFT: Sets the **Move** as the preferred action in drag & drop sesión
        else if (L_SHIFT)
        {
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
        }
    }
}
//! [keyEvent]
