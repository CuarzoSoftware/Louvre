#include "Popup.h"

#include <LCursor.h>
#include <LOutput.h>
#include <LPositioner.h>
#include <LToplevelRole.h>

#include "Global.h"
#include "Surface.h"

Popup::Popup(const void *params) : LPopupRole(params) {}

void Popup::configureRequest() {
  if (G::searchFullscreenParent((Surface *)surface()->parent()))
    setBounds(cursor()->output() != nullptr ? cursor()->output()->rect()
                                            : LRect());
  else
    setBounds(cursor()->output() != nullptr
                  ? cursor()->output()->rect() +
                        LRect(0, TOPBAR_HEIGHT, 0, -TOPBAR_HEIGHT)
                  : LRect());

  if (surface()->parent()->toplevel() &&
      surface()->parent()->toplevel()->pendingConfiguration().state.check(
          LToplevelRole::Maximized | LToplevelRole::Fullscreen)) {
    configureRect(calculateUnconstrainedRect(&bounds().pos()));
    return;
  }

  configureRect(calculateUnconstrainedRect());
}
