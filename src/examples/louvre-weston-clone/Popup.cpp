#include "Popup.h"
#include "LCompositor.h"
#include "LCursor.h"
#include "LOutput.h"
#include "LPositioner.h"

Popup::Popup(Params *params) : LPopupRole(params)
{

}

void Popup::configureRequest()
{
    setPositionerBoundsC(LRect(
                             compositor()->cursor()->output()->rectC().x(),
                             compositor()->cursor()->output()->rectC().y() + 32 * compositor()->globalScale(),
                             compositor()->cursor()->output()->rectC().w(),
                             compositor()->cursor()->output()->rectC().h() - 32 * compositor()->globalScale()
                             ));

    LPoint p = rolePosC() - surface()->parent()->posC();
    configureC(LRect(p,positioner().sizeC()));
    compositor()->raiseSurface(surface());
}
