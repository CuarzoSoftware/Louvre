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
#include "Workspace.h"

Toplevel::Toplevel(Params *params) : LToplevelRole(params),
    blackFullscreenBackground(0.f, 0.f, 0.f, 1.f),
    capture()
{}

Toplevel::~Toplevel()
{
    destructorCalled = true;

    if (fullscreenOutput && fullscreenOutput->animatedFullscreenToplevel == this)
        fullscreenOutput->workspaceAnim->stop();

    unsetFullscreen();

    if (capture.texture())
        delete capture.texture();

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

    if (prevRect.y() < TOPBAR_HEIGHT)
        prevRect.setY(TOPBAR_HEIGHT);

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

    if (dstOutput->animatedFullscreenToplevel)
        return;

    prevRect = LRect(surface()->pos(), windowGeometry().size());
    dstRect = LRect(dstOutput->pos(), dstOutput->size());

    fullscreenOutput = dstOutput;
    configure(dstRect.size(), Activated | Fullscreen);

    LBox box = surf->getView()->boundingBox();
    prevBoundingRect = LRect(box.x1,
                             box.y1,
                             box.x2 - box.x1,
                             box.y2 - box.y1);

    LTexture *captureTexture = surf->renderThumbnail();

    LTexture *old = capture.texture();

    capture.setTexture(captureTexture);
    capture.setBufferScale(2);
    capture.setPos(- windowGeometry().pos().x(), - windowGeometry().pos().y());

    if (old)
        delete old;
}

void Toplevel::unsetFullscreenRequest()
{
    if (!fullscreen() || !fullscreenOutput)
        return;

    if (fullscreenOutput->animatedFullscreenToplevel)
        return;

    Surface *surf = (Surface*)surface();

    LTexture *captureTexture = surf->renderThumbnail();

    LTexture *old = capture.texture();

    capture.setVisible(false);
    capture.setTexture(captureTexture);
    capture.setBufferScale(2);
    capture.setPos(surf->getView()->pos() - fullscreenOutput->pos());

    if (old)
        delete old;

    configure(prevRect.size(), NoState);

}

void Toplevel::fullscreenChanged()
{
    Surface *surf = (Surface*)surface();

    if (fullscreen())
    {
        /*
        if (!fullscreenOutput)
        {
            configure(prevRect.size(), Activated);
            return;
        }*/

        fullscreenOutput->animatedFullscreenToplevel = this;

        surf->sendOutputEnterEvent(fullscreenOutput);

        surf->getView()->setOpacity(0.f);
        capture.setParent(&blackFullscreenBackground);
        capture.enableParentOffset(true);
        capture.enableParentOpacity(false);
        capture.enableParentScaling(true);
        capture.enableDstSize(true);
        capture.setDstSize(fullscreenOutput->size() + (capture.nativePos() * - 2));
        capture.setOpacity(1.f);
        capture.setVisible(true);

        blackFullscreenBackground.setPos(prevBoundingRect.pos());
        blackFullscreenBackground.setSize(fullscreenOutput->size());
        blackFullscreenBackground.setOpacity(0.01f);

        blackFullscreenBackground.setParent(&G::compositor()->overlayLayer);
        blackFullscreenBackground.enableParentOffset(false);
        blackFullscreenBackground.setVisible(true);
        blackFullscreenBackground.enableScaling(true);

        LSizeF sVector;
        sVector.setW(Float32(prevBoundingRect.size().w()) / Float32(fullscreenOutput->size().w()));
        sVector.setH(Float32(prevBoundingRect.size().h()) / Float32(fullscreenOutput->size().h()));

        blackFullscreenBackground.setScalingVector(sVector);

        G::reparentWithSubsurfaces(surf, &blackFullscreenBackground);
        G::enableParentScalingChildren(&blackFullscreenBackground, true);
        surf->getView()->enableParentOffset(true);
        surf->setPos(0, 0);
        surf->getView()->setVisible(true);

        fullscreenWorkspace = new Workspace(fullscreenOutput, this);

        // If the current workspace is the desktop, move the desktop views into it
        if (fullscreenOutput->currentWorkspace == fullscreenOutput->workspaces.front())
            fullscreenOutput->currentWorkspace->stealChildren();

        if (decoratedView)
            decoratedView->fullscreenTopbarVisibility = 0.f;

        fullscreenOutput->setWorkspace(fullscreenWorkspace, 600, 3.f);
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

    Surface *surf = (Surface*)surface();

    if (surf->mapped() && surf->firstMap)
    {
        surf->firstMap = false;
        surf->view->setVisible(true);
    }

    if (!fullscreen() && rolePos().y() < TOPBAR_HEIGHT)
        surface()->setY(TOPBAR_HEIGHT);
}

void Toplevel::geometryChanged()
{
    if (this == seat()->pointer()->resizingToplevel())
        seat()->pointer()->updateResizingToplevelPos();

    if (decoratedView)
    {
        decoratedView->updateTitle();
        decoratedView->updateGeometry();
    }
}

void Toplevel::activatedChanged()
{
    if (decoratedView)
        decoratedView->updateGeometry();

    if (activated())
        seat()->keyboard()->setFocus(surface());
}

void Toplevel::appIdChanged()
{
    LLog::debug("APP ID %s", appId());
}

void Toplevel::titleChanged()
{
    if (decoratedView)
        decoratedView->updateTitle();
}

void Toplevel::unsetFullscreen()
{
    if (!fullscreenOutput)
        return;

    if (destructorCalled)
    {
        if (fullscreenWorkspace)
        {
            Workspace *prev = *std::prev(fullscreenWorkspace->outputLink);

            if (!fullscreenOutput->animatedFullscreenToplevel && fullscreenOutput->currentWorkspace == fullscreenWorkspace)
                fullscreenOutput->setWorkspace(prev, 600);

            delete fullscreenWorkspace;
        }
        return;
    }

    Surface *surf = (Surface*)surface();

    if (decoratedView)
        decoratedView->updateGeometry();

    LTexture *oldTex = captureUnfullscreen.texture();
    LTexture *newCapture = surf->renderThumbnail();
    captureUnfullscreen.setTexture(newCapture);
    captureUnfullscreen.setBufferScale(2);
    captureUnfullscreen.enableParentScaling(true);
    captureUnfullscreen.enableParentOpacity(false);
    captureUnfullscreen.enableDstSize(true);
    captureUnfullscreen.setDstSize(fullscreenOutput->size());
    captureUnfullscreen.setPos(0, 0);
    captureUnfullscreen.setParent(&blackFullscreenBackground);
    captureUnfullscreen.setOpacity(0.f);

    if (oldTex)
        delete oldTex;

    fullscreenOutput->animatedFullscreenToplevel = this;

    surf->getView()->setVisible(false);
    capture.setParent(&blackFullscreenBackground);
    capture.enableParentOffset(true);
    capture.enableParentOpacity(true);
    capture.enableParentScaling(true);
    capture.enableDstSize(true);
    capture.setDstSize(fullscreenOutput->size());
    capture.setOpacity(1.f);
    capture.setVisible(true);

    blackFullscreenBackground.setPos(fullscreenOutput->pos());
    blackFullscreenBackground.setSize(fullscreenOutput->size());
    blackFullscreenBackground.setOpacity(1.f);

    blackFullscreenBackground.setParent(&G::compositor()->overlayLayer);
    blackFullscreenBackground.enableParentOffset(false);
    blackFullscreenBackground.setVisible(true);
    blackFullscreenBackground.enableScaling(true);

    blackFullscreenBackground.setScalingVector(LSizeF(1.f, 1.f));

    G::reparentWithSubsurfaces(surf, &blackFullscreenBackground);
    G::enableParentScalingChildren(&blackFullscreenBackground, true);

    fullscreenOutput->setWorkspace(fullscreenOutput->workspaces.front(), 600, 3.f);
}

void Toplevel::preferredDecorationModeChanged()
{
    if (preferredDecorationMode() == decorationMode() || preferredDecorationMode() == 0)
        return;

    setDecorationMode((DecorationMode)preferredDecorationMode());
    configure(states());
}

