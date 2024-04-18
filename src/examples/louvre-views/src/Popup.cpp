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
        setPositionerBounds(cursor()->output() != nullptr ? cursor()->output()->rect() : LRect());
    else
        setPositionerBounds(cursor()->output() != nullptr ? cursor()->output()->rect() + LRect(0, TOPBAR_HEIGHT, 0, -TOPBAR_HEIGHT) : LRect());

    configure(calculateUnconstrainedRect());
}
