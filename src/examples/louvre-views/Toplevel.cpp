#include "Toplevel.h"
#include "Global.h"
#include <LCursor.h>
#include <LOutput.h>
#include <LSurface.h>

Toplevel::Toplevel(Params *params) : LToplevelRole(params) {}

void Toplevel::setMaximizedRequest()
{
    configure(cursor()->output()->size().w(),
              cursor()->output()->size().h() - TOPBAR_HEIGHT,
              states() | Maximized);
}

void Toplevel::maximizedChanged()
{
    if (maximized())
        surface()->setPos(cursor()->output()->pos() + LPoint(0, TOPBAR_HEIGHT));
}
