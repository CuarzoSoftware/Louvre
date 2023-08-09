#include <LAnimation.h>
#include <LCursor.h>
#include <LSurface.h>
#include <LLog.h>

#include "Compositor.h"
#include "Toplevel.h"
#include "Global.h"
#include "Surface.h"
#include "ToplevelView.h"
#include "Output.h"
#include "Topbar.h"

Toplevel::Toplevel(Params *params) : LToplevelRole(params) {}

Toplevel::~Toplevel()
{
    unsetFullscreen();

    if (decoratedView)
    {
        delete decoratedView;
        decoratedView = nullptr;
    }
}

const LPoint &Toplevel::rolePos() const
{
    if (decoratedView && !fullscreen())
        m_rolePos = surface()->pos() + LPoint(0, TOPLEVEL_TOPBAR_HEIGHT);
    else
        m_rolePos = surface()->pos() - windowGeometry().topLeft();

    return m_rolePos;
}

void Toplevel::configureRequest()
{
    if (!preferredDecorationMode())
        setDecorationMode(ServerSide);
    else
        setDecorationMode((DecorationMode) preferredDecorationMode());

    Surface *surf = (Surface*) surface();

    if (surf->firstMap)
        configure(0, Activated);
    else
        configure(0, states() | Activated);
}

void Toplevel::startResizeRequest(ResizeEdge edge)
{
    // Disable interactive resizing in fullscreen mode
    if (fullscreen())
        return;

    G::enableDocks(false);
    seat()->pointer()->startResizingToplevel(this, edge, LPointer::EdgeDisabled, TOPBAR_HEIGHT);
}

void Toplevel::startMoveRequest()
{
    // Disable interactive moving in fullscreen mode
    if (fullscreen())
        return;

    G::enableDocks(false);
    seat()->pointer()->startMovingToplevel(this, LPointer::EdgeDisabled, TOPBAR_HEIGHT);
}

void Toplevel::setMaximizedRequest()
{
    // Already in maximized mode
    if (maximized())
        return;

    Output *output = (Output*)cursor()->output();

    if (!fullscreen())
        prevRect = LRect(surface()->pos(), windowGeometry().size());

    if (maxSize().w() == 0 || maxSize().w() >= output->rect().w())
        dstRect.setW(output->rect().w());
    else
        dstRect.setW(maxSize().w());

    if (maxSize().h() == 0 || maxSize().h() >= output->rect().h() - TOPBAR_HEIGHT)
        dstRect.setH(output->rect().h() - TOPBAR_HEIGHT);
    else
        dstRect.setH(maxSize().h());

    dstRect.setPos(output->pos() + LPoint(0, TOPBAR_HEIGHT) + (output->size() - LSize(0, TOPBAR_HEIGHT) - dstRect.size()) / 2);

    if (decoratedView)
        dstRect.setSize(dstRect.size() + LSize(2, 2 - TOPLEVEL_TOPBAR_HEIGHT));

    configure(dstRect.size(), Activated | Maximized);
}

void Toplevel::unsetMaximizedRequest()
{
    if (!maximized())
        return;

    configure(prevRect.size(), states() & ~Maximized);
}

void Toplevel::maximizedChanged()
{
    if (maximized())
        surface()->setPos(dstRect.pos());
    else
    {
        if (!seat()->pointer()->movingToplevel() && !seat()->pointer()->resizingToplevel())
            surface()->setPos(prevRect.pos());
    }

    surface()->raise();
    G::compositor()->updatePointerBeforePaint = true;
}

void Toplevel::setFullscreenRequest(LOutput *output)
{
    Surface *surf = (Surface*)surface();

    // Already in fullscreen mode
    if (fullscreen() || !surf || surf->firstMap)
        return;

    Output *dstOutput;

    // Clients can request to maximize a toplevel on a specific output
    if (output)
        dstOutput = (Output*)output;

    // If no output is specified we use the output where the cursor is located
    else
        dstOutput = (Output*)cursor()->output();

    // If there is already another fullscreen toplevel on that output we reject the request
    if (dstOutput->fullscreenToplevel)
        return;

    prevRect = LRect(surface()->pos(), windowGeometry().size());
    dstRect = LRect(dstOutput->pos(), dstOutput->size());

    fullscreenOutput = dstOutput;
    configure(dstRect.size(), Activated | Fullscreen);
}

void Toplevel::unsetFullscreenRequest()
{
    if (!fullscreen())
        return;

    configure(prevRect.size(), Activated);
}

void Toplevel::fullscreenChanged()
{
    Surface *surf = (Surface*)surface();

    if (fullscreen())
    {
        fullscreenOutput->fullscreenToplevel = this;
        fullscreenOutput->fullscreenView->setVisible(true);
        fullscreenOutput->fullscreenView->setPos(fullscreenOutput->pos());
        fullscreenOutput->fullscreenView->setSize(fullscreenOutput->size());
        surf->getView()->setParent(fullscreenOutput->fullscreenView);
        surf->getView()->enableParentOffset(false);
        surface()->setPos(dstRect.pos());
        fullscreenOutput->topbar->hide();
        fullscreenOutput->topbar->update();

        LSurface *ls = surface();
        while ((ls = ls->nextSurface()))
        {
            if (ls->parent() == surface())
            {
                Surface *s = (Surface*)ls;
                s->getView()->setParent(fullscreenOutput->fullscreenView);
            }
        }

        surface()->raise();
    }
    else
    {
        unsetFullscreen();
    }

    if (decoratedView)
        decoratedView->updateGeometry();
}

void Toplevel::setMinimizedRequest()
{
    if (fullscreen() || surface()->minimized())
        return;

    surface()->setMinimized(true);
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

void Toplevel::unsetFullscreen()
{
    if (!fullscreenOutput)
        return;

    Surface *surf = (Surface*)surface();

    while (!fullscreenOutput->fullscreenView->children().empty())
        fullscreenOutput->fullscreenView->children().back()->setParent(G::compositor()->surfacesLayer);

    surface()->raise();
    surf->setPos(prevRect.pos());
    fullscreenOutput->fullscreenView->setVisible(false);
    fullscreenOutput->fullscreenToplevel = nullptr;
    fullscreenOutput->topbar->update();
    fullscreenOutput = nullptr;
}

void Toplevel::preferredDecorationModeChanged()
{
    if (preferredDecorationMode() == decorationMode() || preferredDecorationMode() == 0)
        return;

    setDecorationMode((DecorationMode)preferredDecorationMode());
    configure(states());
}

