#include <CZ/Louvre/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/Events/LKeyboardKeyEvent.h>
#include <CZ/Louvre/LKeyboard.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LSeat.h>
#include <CZ/Louvre/LDND.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/LCursor.h>
#include <CZ/Louvre/LOutput.h>
#include <CZ/Louvre/LLauncher.h>
#include <CZ/Louvre/LUtils.h>
#include <unistd.h>

using namespace Louvre;

//! [keyEvent]
void LKeyboard::keyEvent(const LKeyboardKeyEvent &event)
{
    const bool sessionLocked { sessionLockManager()->state() != LSessionLockManager::Unlocked };

    sendKeyEvent(event);

    const bool L_CTRL      { isKeyCodePressed(KEY_LEFTCTRL)  };
    const bool R_CTRL      { isKeyCodePressed(KEY_RIGHTCTRL) };
    const bool L_SHIFT     { isKeyCodePressed(KEY_LEFTSHIFT) };
    const bool L_ALT       { isKeyCodePressed(KEY_LEFTALT)   };
    const bool mods        { L_ALT || L_SHIFT || L_CTRL || R_CTRL };
    const xkb_keysym_t sym { keySymbol(event.keyCode()) };

    if (event.state() == LKeyboardKeyEvent::Released)
    {
        if (event.keyCode() == KEY_ESC && L_CTRL && L_SHIFT)
        {
            compositor()->finish();
            return;
        }
        else if (L_CTRL && !L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Copy);
        else if (!L_CTRL && L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Move);
        else if (!L_CTRL && !L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::NoAction);

        if (sessionLocked)
            return;

        if (event.keyCode() == KEY_F1 && !mods)
            LLauncher::launch("weston-terminal");
        else if (L_CTRL && (sym == XKB_KEY_q || sym == XKB_KEY_Q))
        {
            if (focus())
                focus()->client()->destroyLater();
        }
        else if (L_CTRL && (sym == XKB_KEY_m || sym == XKB_KEY_M))
        {
            if (focus() && focus()->toplevel() && !focus()->toplevel()->fullscreen())
                focus()->setMinimized(true);
        }
        // Screenshot
        else if (L_CTRL && L_SHIFT && event.keyCode() == KEY_3)
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
    }

    // Key pressed
    else
    {
        // CTRL sets Copy as the preferred action in drag & drop session
        if (L_CTRL)
            seat()->dnd()->setPreferredAction(LDND::Copy);

        // SHIFT sets Move as the preferred action in drag & drop session
        else if (L_SHIFT)
            seat()->dnd()->setPreferredAction(LDND::Move);
    }
}
//! [keyEvent]

//! [focusChanged]
void LKeyboard::focusChanged()
{
    /* No default implementation. */
}
//! [focusChanged]
