#include <LKeyboard.h>
#include <LCompositor.h>
#include <LSeat.h>
#include <LDNDManager.h>
#include <LClient.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LLauncher.h>
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

    const bool L_CTRL      { isKeyCodePressed(KEY_LEFTCTRL) };
    const bool L_SHIFT     { isKeyCodePressed(KEY_LEFTSHIFT) };
    const bool mods        { isKeyCodePressed(KEY_LEFTALT) && L_CTRL };
    const xkb_keysym_t sym { keySymbol(keyCode) };

    if (keyState == Released)
    {
        if (keyCode == KEY_F1 && !mods)
            LLauncher::launch("weston-terminal");
        else if (L_CTRL && (sym == XKB_KEY_q || sym == XKB_KEY_Q))
        {
            if (focus())
                focus()->client()->destroy();
        }
        else if (L_CTRL && (sym == XKB_KEY_m || sym == XKB_KEY_M))
        {
            if (focus() && focus()->toplevel() && !focus()->toplevel()->fullscreen())
                focus()->setMinimized(true);
        }
        // Screenshot
        else if (L_CTRL && L_SHIFT && keyCode == KEY_3)
        {
            if (cursor()->output() && cursor()->output()->bufferTexture(0))
            {
                std::filesystem::path path { getenvString("HOME") };

                if (path.empty())
                    return;

                path /= "Desktop/Louvre_Screenshoot_";

                char timeString[32];
                const auto now { std::chrono::system_clock::now() };
                const auto time { std::chrono::system_clock::to_time_t(now) };
                std::strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S.png", std::localtime(&time));

                path += timeString;

                cursor()->output()->bufferTexture(0)->save(path);
            }
        }
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

    // Key pressed
    else
    {
        // CTRL sets Copy as the preferred action in drag & drop session
        if (L_CTRL)
            seat()->dndManager()->setPreferredAction(LDNDManager::Copy);

        // SHIFT sets the Move as the preferred action in drag & drop session
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
