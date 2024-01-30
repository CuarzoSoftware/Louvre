#include <LAnimation.h>
#include <LCursor.h>
#include <LSurface.h>
#include <LLog.h>
#include <unistd.h>

#include "Compositor.h"
#include "Toplevel.h"
#include "Global.h"
#include "Surface.h"
#include "ToplevelView.h"
#include "Output.h"
#include "Workspace.h"

#define WORSPACE_ANIM_MS 600
#define WORSPACE_ANIM_EASE 5.f

Toplevel::Toplevel(Params *params) : LToplevelRole(params),
    blackFullscreenBackground(0.f, 0.f, 0.f, 1.f),
    capture(nullptr, &blackFullscreenBackground),
    animView(nullptr, &G::compositor()->overlayLayer)
{
    blackFullscreenBackground.enableParentOffset(false);
    blackFullscreenBackground.setVisible(false);

    capture.setBufferScale(2);
    capture.enableParentOpacity(false);
    capture.enableDstSize(true);
}

Toplevel::~Toplevel()
{
    destructorCalled = true;

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
    if (decoratedView)
    {
        m_rolePos = surface()->pos();

        if (!fullscreen() && !animScene)
            m_rolePos += LPoint(0, TOPLEVEL_TOPBAR_HEIGHT);
    }
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

    if (surf()->firstMap)
        configure(0, Activated);
    else
        configure(0, pendingStates() | Activated);
}

void Toplevel::startResizeRequest(ResizeEdge edge)
{
    // Disable interactive resizing in fullscreen mode
    if (fullscreen())
        return;

    G::enableDocks(false);
    seat()->pointer()->startResizingToplevel(this,
                                             edge,
                                             cursor()->pos(),
                                             LSize(128, 128),
                                             LPointer::EdgeDisabled,
                                             TOPBAR_HEIGHT);
}

void Toplevel::startMoveRequest()
{
    // Disable interactive moving in fullscreen mode
    if (fullscreen())
        return;

    G::enableDocks(false);
    seat()->pointer()->startMovingToplevel(this, cursor()->pos(), LPointer::EdgeDisabled, TOPBAR_HEIGHT);
}

void Toplevel::setMaximizedRequest()
{
    // Already in maximized mode
    if (maximized())
    {
        configure(pendingStates());
        return;
    }

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
    {
        configure(pendingStates());
        return;
    }

    configure(prevRect.size(), pendingStates() & ~Maximized);
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

    if (animScene)
    {
        configure(pendingStates());
        return;
    }

    // Already in fullscreen mode
    if (fullscreen() || !surf || surf->firstMap)
    {
        configure(pendingStates());
        return;
    }

    Output *dstOutput;

    // Clients can request to maximize a toplevel on a specific output
    if (output)
        dstOutput = (Output*)output;

    // If no output is specified we use the output where the cursor is located
    else
        dstOutput = (Output*)cursor()->output();

    if (dstOutput->animatedFullscreenToplevel)
    {
        configure(pendingStates());
        return;
    }

    prevRect = LRect(surface()->pos(), windowGeometry().size());
    dstRect = LRect(dstOutput->pos(), dstOutput->size());

    fullscreenOutput = dstOutput;
    prevStates = states();
    configure(dstRect.size(), Activated | Fullscreen);

    LBox box = surf->getView()->boundingBox();
    prevBoundingRect = LRect(box.x1,
                             box.y1,
                             box.x2 - box.x1,
                             box.y2 - box.y1);

    if (capture.texture())
        delete capture.texture();

    capture.setTexture(surf->renderThumbnail(&captureTransRegion));
    capture.setPos(- windowGeometry().pos().x(), - windowGeometry().pos().y());
}

void Toplevel::unsetFullscreenRequest()
{
    if (!fullscreen() || !fullscreenOutput || fullscreenOutput->animatedFullscreenToplevel)
    {
        configure(pendingStates());
        return;
    }

    if (capture.texture())
        delete capture.texture();

    LSceneView tmp(fullscreenOutput->sizeB(), fullscreenOutput->scale());
    blackFullscreenBackground.setSize(fullscreenOutput->size());
    blackFullscreenBackground.setParent(&tmp);
    blackFullscreenBackground.enableParentOffset(true);
    blackFullscreenBackground.setVisible(true);
    blackFullscreenBackground.setPos(0);
    LPoint prevPos = surf()->pos();
    bool parentOffsetEnabled = surf()->getView()->parentOffsetEnabled();
    G::reparentWithSubsurfaces(surf(), &blackFullscreenBackground, true);
    surf()->setPos(0, 0);
    tmp.setPos(fullscreenOutput->pos());
    tmp.render();
    capture.setTexture(tmp.texture()->copyB());
    capture.setBufferScale(tmp.bufferScale());
    surf()->setPos(prevPos);
    surf()->getView()->enableParentOffset(parentOffsetEnabled);
    G::reparentWithSubsurfaces(surf(), &fullscreenWorkspace->surfaces, true);
    capture.setVisible(false);
    configure(prevRect.size(), prevStates);
}

void Toplevel::fullscreenChanged()
{
    if (fullscreen())
    {
        if (!fullscreenOutput)
        {
            configure(pendingStates() &~ Fullscreen);
            return;
        }

        if (animScene)
            delete animScene;

        animScene = new LSceneView(fullscreenOutput->size() * 0.75f, 0.75f);
        quickUnfullscreen = false;
        fullscreenOutput->animatedFullscreenToplevel = this;
        surf()->sendOutputEnterEvent(fullscreenOutput);

        capture.setParent(&G::compositor()->overlayLayer);
        capture.setDstSize(capture.texture()->sizeB() / capture.bufferScale());
        capture.setOpacity(1.f);
        capture.setVisible(true);
        capture.enableParentOpacity(false);
        capture.setTranslucentRegion(&captureTransRegion);

        blackFullscreenBackground.setParent(animScene);
        blackFullscreenBackground.setPos(0);
        blackFullscreenBackground.setSize(fullscreenOutput->size());
        blackFullscreenBackground.setOpacity(1.f);
        blackFullscreenBackground.setVisible(true);

        G::reparentWithSubsurfaces(surf(), &blackFullscreenBackground);
        surf()->getView()->enableParentOpacity(true);
        surf()->getView()->setOpacity(1.f);
        surf()->getView()->setVisible(true);
        surf()->setPos(0, 0);

        LRegion empty;
        surf()->view->enableCustomTranslucentRegion(true);
        surf()->view->setCustomTranslucentRegion(&empty);

        animView.enableDstSize(true);
        animView.insertAfter(&capture, true);
        animView.setTranslucentRegion(nullptr);
        animView.setVisible(false);

        fullscreenWorkspace = new Workspace(fullscreenOutput, this, fullscreenOutput->currentWorkspace);

        // If the current workspace is the desktop, move the desktop views into it
        if (fullscreenOutput->currentWorkspace == fullscreenOutput->workspaces.front())
            fullscreenOutput->currentWorkspace->stealChildren();

        if (decoratedView)
            decoratedView->fullscreenTopbarVisibility = 0.f;

        fullscreenOutput->setWorkspace(fullscreenWorkspace, WORSPACE_ANIM_MS, WORSPACE_ANIM_EASE);
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
    if (fullscreen() || surface()->minimized() || animScene)
        return;

    surface()->setMinimized(true);
}

void Toplevel::decorationModeChanged()
{
    if (decorationMode() == ClientSide)
    {
        LView *prevParent = decoratedView->parent();
        delete decoratedView;
        decoratedView = nullptr;

        surf()->view->setPrimary(true);
        surf()->view->enableParentClipping(false);
        surf()->view->enableCustomPos(false);
        surf()->view->setParent(prevParent);
        surf()->requestNextFrame(false);
    }
    else
    {
        decoratedView = new ToplevelView(this);
        decoratedView->setVisible(surf()->view->visible());
        surf()->view->enableParentClipping(true);
        surf()->view->enableCustomPos(true);
        surf()->view->enableParentOffset(true);
    }

    if (!fullscreen() && rolePos().y() < TOPBAR_HEIGHT)
        surface()->setY(TOPBAR_HEIGHT);
}

void Toplevel::geometryChanged()
{
    if (resizing())
        updateResizingPos();

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
    {
        seat()->keyboard()->setFocus(surface());

        if (!fullscreen() && !surf()->parent())
        {
            Output *o = (Output*)cursor()->output();

            if (o->currentWorkspace != o->workspaces.front())
                o->setWorkspace(o->workspaces.front(), 600.f, 4.f);
        }
    }
}

void Toplevel::titleChanged()
{
    if (decoratedView)
        decoratedView->updateTitle();
}

void Toplevel::unsetFullscreen()
{
    if (surf())
        surf()->view->enableCustomTranslucentRegion(false);

    if (!fullscreenOutput)
        return;

    if (destructorCalled || quickUnfullscreen)
    {
        if (fullscreenWorkspace)
        {
            Workspace *prev = *std::prev(fullscreenWorkspace->outputLink);
            fullscreenOutput->workspaceAnim.stop();
            fullscreenOutput->animatedFullscreenToplevel = nullptr;
            delete fullscreenWorkspace;
            fullscreenWorkspace = nullptr;
            fullscreenOutput->setWorkspace(prev, 600.f, 4.0f, 0.5f);
            fullscreenOutput = nullptr;
        }
        return;
    }

    if (decoratedView)
        decoratedView->updateGeometry();

    if (animScene)
        delete animScene;

    animScene = new LSceneView(fullscreenOutput->size() * 0.75f, 0.75f);
    animScene->setPos(fullscreenOutput->pos());
    G::reparentWithSubsurfaces(surf(), animScene, true);
    fullscreenOutput->animatedFullscreenToplevel = this;

    capture.enableDstSize(true);
    capture.setDstSize(fullscreenOutput->size());
    capture.setOpacity(1.f);
    capture.setVisible(true);
    capture.enableParentOpacity(false);

    animView.setParent(&G::compositor()->overlayLayer);
    capture.insertAfter(&animView);

    animView.setTranslucentRegion(nullptr);
    animView.setOpacity(1.f);

    fullscreenOutput->setWorkspace(fullscreenOutput->workspaces.front(), WORSPACE_ANIM_MS, WORSPACE_ANIM_EASE);
    G::scene()->mainView()->damageAll(fullscreenOutput);
}

void Toplevel::preferredDecorationModeChanged()
{
    if (preferredDecorationMode() == decorationMode() || preferredDecorationMode() == 0)
        return;

    setDecorationMode((DecorationMode)preferredDecorationMode());
    configure(pendingStates());
}

