#include <LLog.h>
#include <LCursor.h>
#include <LCompositor.h>
#include "Seat.h"
#include "Output.h"

Seat::Seat(const void *params) noexcept : LSeat(params){}

void Seat::enabledChanged()
{
    if (!enabled())
        return;

    for (Output *o : (std::vector<Output*>&)compositor()->outputs())
    {
        o->fullDamage();
        o->repaint();
    }

    LLog::log("[louvre-weston-clone] Session restored.");

    if (cursor())
    {
        cursor()->setVisible(false);
        cursor()->setVisible(true);
    }
}
