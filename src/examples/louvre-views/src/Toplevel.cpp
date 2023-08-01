#include <LCursor.h>
#include <LOutput.h>
#include <LSurface.h>
#include <LLog.h>

#include "Compositor.h"
#include "Toplevel.h"
#include "Global.h"
#include "Surface.h"
#include "ToplevelView.h"

Toplevel::Toplevel(Params *params) : LToplevelRole(params) {}

Toplevel::~Toplevel()
{
    if (decoratedView)
    {
        delete decoratedView;
        decoratedView = nullptr;
    }
}

const LPoint &Toplevel::rolePos() const
{
    if (decorationMode() == ClientSide)
        m_rolePos = surface()->pos() - windowGeometry().topLeft();
    else
        m_rolePos = surface()->pos() + LPoint(0, TOPLEVEL_TOPBAR_HEIGHT);

    return m_rolePos;
}

void Toplevel::configureRequest()
{
    if (!preferredDecorationMode())
        setDecorationMode(ServerSide);
    else
        setDecorationMode((DecorationMode) preferredDecorationMode());

    configure(0, states() | Activated);
}

void Toplevel::startResizeRequest(ResizeEdge edge)
{
    seat()->pointer()->startResizingToplevel(this, edge, LPointer::EdgeDisabled, TOPBAR_HEIGHT);
}

void Toplevel::startMoveRequest()
{
    seat()->pointer()->startMovingToplevel(this, LPointer::EdgeDisabled, TOPBAR_HEIGHT);
}

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

void Toplevel::decorationModeChanged()
{
    if (decorationMode() == ClientSide)
    {
        delete decoratedView;
        decoratedView = nullptr;
    }
    else
    {
        decoratedView = new ToplevelView(this);
    }
}

void Toplevel::geometryChanged()
{
    if (this == seat()->pointer()->resizingToplevel())
        seat()->pointer()->updateResizingToplevelPos();

    if (decoratedView)
        decoratedView->updateGeometry();
}

void Toplevel::activatedChanged()
{
    if (decoratedView)
        decoratedView->updateGeometry();
}

void Toplevel::preferredDecorationModeChanged()
{
    if (preferredDecorationMode() == decorationMode() || preferredDecorationMode() == 0)
        return;

    setDecorationMode((DecorationMode)preferredDecorationMode());
    configure(states());
}

