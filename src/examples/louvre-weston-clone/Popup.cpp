#include <LCursor.h>
#include <LPositioner.h>
#include <LSurface.h>
#include "Output.h"
#include "Popup.h"

Popup::Popup(void *params) : LPopupRole(params) {}

void Popup::configureRequest()
{
    Output *output = (Output*)cursor()->output();

    if (output->fullscreenSurface)
        setPositionerBounds(output->rect());
    else
        setPositionerBounds(LRect(
            output->rect().x(),
            output->rect().y() + 32,
            output->rect().w(),
            output->rect().h() - 32
            ));

    LPoint p = rolePos() - surface()->parent()->pos();
    configure(LRect(p, positioner().size()));
    surface()->raise();
}
