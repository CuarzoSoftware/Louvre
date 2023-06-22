#include "Popup.h"
#include "LCompositor.h"
#include "LCursor.h"
#include "LOutput.h"
#include "LPositioner.h"
#include "Output.h"

Popup::Popup(Params *params) : LPopupRole(params)
{

}

void Popup::configureRequest()
{
    Output *output = (Output*)cursor()->output();

    if (output->fullscreenSurface)
    {
        setPositionerBoundsC(cursor()->output()->rectC());
    }
    else
    {
        setPositionerBoundsC(LRect(
            cursor()->output()->rectC().x(),
            cursor()->output()->rectC().y() + 32 * compositor()->globalScale(),
            cursor()->output()->rectC().w(),
            cursor()->output()->rectC().h() - 32 * compositor()->globalScale()
            ));
    }

    LPoint p = rolePosC() - surface()->parent()->posC();
    configureC(LRect(p, positioner().sizeC()));
    compositor()->raiseSurface(surface());
}
