#include <LToplevelRole.h>
#include <LPositioner.h>
#include <LCursor.h>
#include <LOutput.h>
#include "Popup.h"
#include "Surface.h"
#include "Global.h"

Popup::Popup(const void *params) : LPopupRole(params) {}

void Popup::configureRequest()
{    
    if (G::searchFullscreenParent((Surface*)surface()->parent()))
        setBounds(cursor()->output() != nullptr ? cursor()->output()->rect() : LRect());
    else
        setBounds(cursor()->output() != nullptr ? cursor()->output()->rect() + LRect(0, TOPBAR_HEIGHT, 0, -TOPBAR_HEIGHT) : LRect());

    if (surface()->parent()->toplevel() && surface()->parent()->toplevel()->pendingConfiguration().state.check(LToplevelRole::Maximized | LToplevelRole::Fullscreen))
    {
        configureRect(calculateUnconstrainedRect(&bounds().pos()));
        return;
    }

    configureRect(calculateUnconstrainedRect());
}
