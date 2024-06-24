#include <LCursor.h>
#include <LPositioner.h>
#include <LSurface.h>
#include "Output.h"
#include "Popup.h"

void Popup::configureRequest()
{
    Output *output { (Output*)cursor()->output() };

    if (output && output->fullscreenSurface)
        setBounds(output != nullptr ? output->rect() : LRect());
    else
        setBounds(output != nullptr ? LRect(
            output->rect().x(),
            output->rect().y() + 32,
            output->rect().w(),
            output->rect().h() - 32
            ) : LRect());

    configureRect(calculateUnconstrainedRect());
}
