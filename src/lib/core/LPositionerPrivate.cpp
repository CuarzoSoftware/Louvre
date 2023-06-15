#include <private/LPositionerPrivate.h>
#include <LCompositor.h>

void LPositioner::LPositionerPrivate::updateGlobalScale()
{
    data.sizeC = data.sizeS * compositor()->globalScale();
    data.anchorRectC = data.anchorRectS * compositor()->globalScale();
    data.offsetC = data.offsetS * compositor()->globalScale();
    data.parentSizeC = data.parentSizeS * compositor()->globalScale();
}
