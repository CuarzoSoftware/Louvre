#include "Seat.h"

#include <LCompositor.h>
#include <LCursor.h>
#include <LLog.h>

#include "Output.h"

void Seat::enabledChanged() {
  /* Session restored (user returned from another TTY) */
  if (enabled()) {
    for (Output *o : (std::vector<Output *> &)compositor()->outputs()) {
      o->fullDamage();
      o->repaint();
    }
  }
}
