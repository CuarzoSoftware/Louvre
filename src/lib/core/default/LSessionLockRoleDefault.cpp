#include <LOutput.h>
#include <LSessionLockRole.h>
#include <LSurface.h>

using namespace Louvre;

//! [rolePos]
const LPoint &LSessionLockRole::rolePos() const {
  if (exclusiveOutput()) return exclusiveOutput()->pos();

  return surface()->pos();
}
//! [rolePos]
