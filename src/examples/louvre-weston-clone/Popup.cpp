#include <LCursor.h>
#include <LPositioner.h>
#include <LSurface.h>
#include "Output.h"
#include "Popup.h"

Popup::Popup(const void *params) noexcept : LPopupRole(params) {}

void Popup::configureRequest()
{
    Output *output { (Output*)cursor()->output() };

    if (output && output->fullscreenSurface)
        setPositionerBounds(output != nullptr ? output->rect() : LRect());
    else
        setPositionerBounds(output != nullptr ? LRect(
            output->rect().x(),
            output->rect().y() + 32,
            output->rect().w(),
            output->rect().h() - 32
            ) : LRect());

    configure(calculateUnconstrainedRect());
}
