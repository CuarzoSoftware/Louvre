#include "Popup.h"
#include "LCompositor.h"
#include "LCursor.h"
#include "LOutput.h"
#include "LPositioner.h"
#include "Output.h"

Popup::Popup(Params *params) : LPopupRole(params) {}

void Popup::configureRequest()
{
    Output *output = (Output*)cursor()->output();

    if (output->fullscreenSurface)
    {
        setPositionerBounds(output->rect());
    }
    else
    {
        setPositionerBounds(LRect(
            output->rect().x(),
            output->rect().y() + 32,
            output->rect().w(),
            output->rect().h() - 32
            ));
    }

    LPoint p = rolePos() - surface()->parent()->pos();
    configure(LRect(p, positioner().size()));
    compositor()->raiseSurface(surface());
}
