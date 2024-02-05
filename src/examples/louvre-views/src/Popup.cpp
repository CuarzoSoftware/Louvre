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
        setPositionerBounds(cursor()->output()->rect());
    else
        setPositionerBounds(cursor()->output()->rect() + LRect(0, TOPBAR_HEIGHT, 0, -TOPBAR_HEIGHT));

    LPoint relativePosition = rolePos() - surface()->parent()->pos();

    configure(LRect(relativePosition, positioner().unconstrainedSize()));
}
