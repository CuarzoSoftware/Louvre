#include <LBackgroundBlur.h>

using namespace Louvre;

//! [propsChanged]
void LBackgroundBlur::propsChanged(LBitset<PropChanges> changes,
                                   const Props &prevProps) {
  L_UNUSED(changes)
  L_UNUSED(prevProps)
}
//! [propsChanged]

//! [configureRequest]
void LBackgroundBlur::configureRequest() {
  /* Example

  configureState(Enabled);
  configureColorHint(Light); */
}
//! [configureRequest]
