#include <CZ/Louvre/LBackgroundBlur.h>

using namespace Louvre;

//! [propsChanged]
void LBackgroundBlur::propsChanged(CZBitset<PropChanges> changes, const Props &prevProps)
{
    L_UNUSED(changes)
    L_UNUSED(prevProps)
}
//! [propsChanged]

//! [configureRequest]
void LBackgroundBlur::configureRequest()
{
    /* Example

    configureState(Enabled);
    configureColorHint(Light); */
}
//! [configureRequest]
