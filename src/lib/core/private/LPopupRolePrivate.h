#ifndef LPOPUPROLEPRIVATE_H
#define LPOPUPROLEPRIVATE_H

#include <LPopupRole.h>

using namespace Louvre;

struct LPopupRole::Params {
  LResource *popup;
  LSurface *surface;
  LPositioner *positioner;
};

#endif  // LPOPUPROLEPRIVATE_H
