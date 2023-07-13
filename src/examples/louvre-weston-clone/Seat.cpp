#include "Seat.h"
#include "Output.h"
#include "Compositor.h"
#include <LLog.h>
#include <LCursor.h>

Seat::Seat(Params *params) : LSeat(params){}

void Seat::seatEnabled()
{
    for (Output *o : (std::list<Output*>&)compositor()->outputs())
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
