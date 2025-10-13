#include <CZ/Louvre/Roles/LBackgroundBlur.h>

using namespace CZ;

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
    configureState(Enabled);
}
//! [configureRequest]
