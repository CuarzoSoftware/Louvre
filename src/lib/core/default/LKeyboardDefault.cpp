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
    sendModifiersEvent(depressed,latched,locked,group);
}
//! [keyModifiersEvent]

//! [keyEvent]
void LKeyboard::keyEvent(UInt32 keyCode, UInt32 keyState)
{

    sendKeyEvent(keyCode, keyState);
    xkb_keysym_t sym = keySymbol(keyCode);

    bool mods = isModActive(XKB_MOD_NAME_ALT) && isModActive(XKB_MOD_NAME_CTRL);

    if (keyState == LIBINPUT_KEY_STATE_RELEASED)
    {

        // F1: Launches weston-terminal.
        if (sym == XKB_KEY_F1 && !mods)
        {
            if (fork()==0)
            {
                setsid();
                system("weston-terminal");
                exit(0);
            }
        }
        // Audio volume -10%
        else if (sym == XKB_KEY_XF86AudioLowerVolume && !mods)
        {
            if (fork()==0)
            {
                system("pactl -- set-sink-volume 0 -10%");
                exit(0);
            }
        }
        // Audio volume +10%
        else if (sym == XKB_KEY_XF86AudioRaiseVolume && !mods)
        {
            if (fork()==0)
            {
                system("pactl -- set-sink-volume 0 +10%");
                exit(0);
            }
        }

        // CTRL + SHIFT + ESC : Kils the compositor.
        else if (sym == XKB_KEY_Escape && isModActive(XKB_MOD_NAME_CTRL) && isModActive(XKB_MOD_NAME_SHIFT))
        {
            LLog::warning("Killing compositor.");
            compositor()->finish();
        }

        // F8 : Unminimize all surfaces.
        else if (sym == XKB_KEY_F8 && !mods)
        {
            for (LSurface *surface : compositor()->surfaces())
                surface->setMinimized(false);

            compositor()->repaintAllOutputs();
        }

        #if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

        // CTRL: Unsets the **Copy** as the preferred action in drag & drop sesión
        else if (sym == XKB_KEY_Control_L)
        {
            if (seat()->dndManager()->preferredAction() == LDNDManager::Copy)
                seat()->dndManager()->setPreferredAction(LDNDManager::NoAction);
        }

        // SHIFT: Unsets the **Move** as the preferred action in drag & drop sesión
        else if (sym == XKB_KEY_Shift_L)
        {
            if (seat()->dndManager()->preferredAction() == LDNDManager::Move)
                seat()->dndManager()->setPreferredAction(LDNDManager::NoAction);
        }

        #endif


    }
    // Key press
    else
    {

        #if LOUVRE_DATA_DEVICE_MANAGER_VERSION >= 3

        // CTRL: Sets the **Copy** as the preferred action in drag & drop sesión
        if (sym == XKB_KEY_Control_L)
        {
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);
        }

        // SHIFT: Sets the **Move** as the preferred action in drag & drop sesión
        else if (sym == XKB_KEY_Shift_L)
        {
            seat()->dndManager()->setPreferredAction(LDNDManager::Move);
        }

        #endif
    }
}
//! [keyEvent]
