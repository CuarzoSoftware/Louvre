#include "Toplevel.h"

#include <LAnimation.h>
#include <LCursor.h>
#include <LLog.h>
#include <LSeat.h>
#include <LSurface.h>
#include <LToplevelMoveSession.h>
#include <LToplevelResizeSession.h>
#include <LTouch.h>
#include <LTouchDownEvent.h>
#include <LTouchPoint.h>
#include <unistd.h>

#include <cassert>

#include "Compositor.h"
#include "Global.h"
#include "Output.h"
#include "Surface.h"
#include "ToplevelView.h"
#include "Workspace.h"

#define WORKSPACE_ANIM_MS 300
#define WORKSPACE_ANIM_EASE 5.f

Toplevel::Toplevel(const void *params)
    : LToplevelRole(params),
      blackFullscreenBackground(0.f, 0.f, 0.f, 1.f),
      captureView(nullptr, &blackFullscreenBackground),
      animView(nullptr, &G::compositor()->overlayLayer) {
  blackFullscreenBackground.enableParentOffset(false);
  blackFullscreenBackground.setVisible(false);

  captureView.setBufferScale(2);
  captureView.enableParentOpacity(false);
  captureView.enableDstSize(true);
  captureView.enablePointerEvents(true);
  captureView.setInputRegion(nullptr);

  resizeSession().setMinSize(LSize(150, 150));
}

Toplevel::~Toplevel() {
  assert(surface() != nullptr);
  destructorCalled = true;
  unsetFullscreen();
}

const LPoint &Toplevel::rolePos() const {
  if (decoratedView) {
    m_rolePos = surface()->pos();

    if (!fullscreen() && !animScene)
      m_rolePos += LPoint(0, extraGeometry().top);
  } else
    m_rolePos = surface()->pos() - windowGeometry().topLeft();

  return m_rolePos;
}

void Toplevel::configureRequest() {
  LOutput *output{cursor()->output()};

  if (output) {
    surface()->sendOutputEnterEvent(output);

    if (supportServerSideDecorations())
      configureBounds(output->availableGeometry().size().w() -
                          (extraGeometry().left + extraGeometry().right),
                      output->availableGeometry().size().h() -
                          (extraGeometry().top + extraGeometry().bottom));
    else
      configureBounds(output->availableGeometry().size());
  }

  configureSize(0, 0);
  configureDecorationMode(ServerSide);
  configureState(pendingConfiguration().state | Activated);
  configureCapabilities(MinimizeCap | FullscreenCap | MaximizeCap);
}

void Toplevel::atomsChanged(LBitset<AtomChanges> changes, const Atoms &prev) {
  if (changes.check(DecorationModeChanged)) decorationModeChanged();

  if (changes.check(StateChanged)) {
    const LBitset<State> stateChanges{state() ^ prev.state};

    if (stateChanges.check(Activated)) activatedChanged();

    if (stateChanges.check(Maximized)) maximizedChanged();

    if (stateChanges.check(Fullscreen)) fullscreenChanged();
  }

  if (changes.check(WindowGeometryChanged)) {
    if (decoratedView) {
      decoratedView->updateTitle();
      decoratedView->updateGeometry();
    }
  }

  surface()->repaintOutputs();
}

void Toplevel::startResizeRequest(const LEvent &triggeringEvent,
                                  LBitset<LEdge> edge) {
  LToplevelRole::startResizeRequest(triggeringEvent, edge);

  if (resizeSession().isActive()) G::enableDocks(false);
}

void Toplevel::setMaximizedRequest() {
  // Already in maximized mode
  if (maximized()) return;

  Output *output{static_cast<Output *>(cursor()->output())};

  if (!output) return;

  setExclusiveOutput(output);

  if (!fullscreen())
    prevRect = LRect(surface()->pos(), windowGeometry().size());

  if (windowGeometry().size().area() == 0)
    prevRect.setSize(output->availableGeometry().size());

  if (prevRect.y() < output->pos().y() + output->availableGeometry().y())
    prevRect.setY(output->pos().y() + output->availableGeometry().y());

  if (maxSize().w() == 0 || maxSize().w() >= output->rect().w())
    dstRect.setW(output->availableGeometry().w());
  else
    dstRect.setW(maxSize().w());

  if (maxSize().h() == 0 || maxSize().h() >= output->availableGeometry().h())
    dstRect.setH(output->availableGeometry().h());
  else
    dstRect.setH(maxSize().h());

  dstRect.setPos(output->pos() + output->availableGeometry().pos() +
                 (output->availableGeometry().size() - dstRect.size()) / 2);

  if (supportServerSideDecorations())
    dstRect.setSize(dstRect.size() - LSize(0, extraGeometry().top));

  configureSize(dstRect.size());
  configureState(Activated | Maximized);
}

void Toplevel::unsetMaximizedRequest() {
  if (!maximized()) return;

  configureSize(prevRect.size());
  configureState(pendingConfiguration().state & ~Maximized);
}

void Toplevel::maximizedChanged() {
  if (maximized())
    surface()->setPos(dstRect.pos());
  else if (seat()->toplevelResizeSessions().empty() &&
           seat()->toplevelMoveSessions().empty()) {
    setExclusiveOutput(nullptr);
    surface()->setPos(prevRect.pos());
  }

  surface()->raise();
  G::compositor()->updatePointerBeforePaint = true;
}

void Toplevel::setFullscreenRequest(LOutput *output) {
  if (surf()->firstMap) {
    requestedFullscreenOnFirstMap = true;
    return;
  }

  // Already in fullscreen mode
  if (animScene || fullscreen() || !surf()) return;

  Output *dstOutput;

  // Clients can request to maximize a toplevel on a specific output
  if (output) dstOutput = (Output *)output;

  // If no output is specified we use the output where the cursor is located
  else
    dstOutput = (Output *)cursor()->output();

  if (!dstOutput) return;

  if (dstOutput->animatedFullscreenToplevel) {
    configureState(pendingConfiguration().state);
    return;
  }

  prevRect = LRect(surface()->pos(), windowGeometry().size());
  dstRect = LRect(dstOutput->pos(), dstOutput->size());

  fullscreenOutput = dstOutput;
  prevStates = state();
  configureSize(dstRect.size());
  configureState(Activated | Fullscreen);

  LBox box = surf()->getView()->boundingBox();
  prevBoundingRect = LRect(box.x1, box.y1, box.x2 - box.x1, box.y2 - box.y1);

  captureTexture.reset(surf()->renderThumbnail(&captureTransRegion));
  captureView.setTexture(captureTexture.get());

  if (!surf()->children().empty()) {
    captureView.setPos(prevBoundingRect.pos());
    captureView.setDstSize(prevBoundingRect.size());
    captureView.setParent(&G::compositor()->overlayLayer);
    captureView.setOpacity(1.f);
    captureView.setVisible(true);
    captureView.setTranslucentRegion(&captureTransRegion);
    G::reparentWithSubsurfaces(surf(), nullptr, true);
    surf()->requestNextFrame(false);
  }
}

void Toplevel::unsetFullscreenRequest() {
  if (!fullscreen() || !fullscreenOutput ||
      fullscreenOutput->animatedFullscreenToplevel) {
    configureState(pendingConfiguration().state);
    return;
  }

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
  captureTexture.reset(tmp.texture()->copy());
  captureView.setTexture(captureTexture.get());
  captureView.setBufferScale(tmp.bufferScale());
  surf()->setPos(prevPos);
  surf()->getView()->enableParentOffset(parentOffsetEnabled);
  G::reparentWithSubsurfaces(surf(), &fullscreenWorkspace->surfaces, true);
  captureView.setVisible(false);
  configureSize(prevRect.size());
  configureState(prevStates);
}

void Toplevel::fullscreenChanged() {
  if (fullscreen()) {
    if (!fullscreenOutput) {
      configureState(pendingConfiguration().state & ~Fullscreen);
      return;
    }

    Float32 quality{fullscreenOutput->scale() * 1.f};
    animScene = std::make_unique<LSceneView>(fullscreenOutput->size() * quality,
                                             quality);
    quickUnfullscreen = false;
    fullscreenOutput->animatedFullscreenToplevel = this;
    surf()->sendOutputEnterEvent(fullscreenOutput);

    captureView.setParent(&G::compositor()->overlayLayer);
    captureView.setDstSize(captureView.texture()->sizeB() /
                           captureView.bufferScale());
    captureView.setOpacity(1.f);
    captureView.setVisible(true);
    captureView.enableParentOpacity(false);
    captureView.setTranslucentRegion(&captureTransRegion);

    blackFullscreenBackground.setParent(animScene.get());
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
    surf()->view.enableCustomTranslucentRegion(true);
    surf()->view.setCustomTranslucentRegion(&empty);

    animView.enableDstSize(true);
    animView.insertAfter(&captureView);
    animView.setTranslucentRegion(nullptr);
    animView.setVisible(false);

    fullscreenWorkspace = new Workspace(fullscreenOutput, this,
                                        fullscreenOutput->currentWorkspace);

    // If the current workspace is the desktop, move the desktop views into it
    if (fullscreenOutput->currentWorkspace ==
        fullscreenOutput->workspaces.front())
      fullscreenOutput->currentWorkspace->stealChildren();

    if (decoratedView) decoratedView->fullscreenTopbarVisibility = 0.f;

    fullscreenOutput->setWorkspace(fullscreenWorkspace, WORKSPACE_ANIM_MS,
                                   WORKSPACE_ANIM_EASE);
  } else {
    unsetFullscreen();
  }

  if (decoratedView) decoratedView->updateGeometry();
}

void Toplevel::setMinimizedRequest() {
  if (fullscreen() || surface()->minimized() || animScene) return;

  surface()->setMinimized(true);
}

void Toplevel::unsetMinimizedRequest() {
  if (surf()->minimizedViews.empty()) return;

  LPointerButtonEvent event(LPointerButtonEvent::Left,
                            LPointerButtonEvent::Released);
  surf()->minimizedViews.front()->pointerButtonEvent(event);
}

void Toplevel::decorationModeChanged() {
  if (decorationMode() == ClientSide) {
    setExtraGeometry({0, 0, 0, 0});
    configureSize(pendingConfiguration().size +
                  LSize(0, TOPLEVEL_TOPBAR_HEIGHT));
    LView *prevParent = decoratedView->parent();
    decoratedView.reset();

    surf()->view.setPrimary(true);
    surf()->view.enableParentClipping(false);
    surf()->view.enableCustomPos(false);
    surf()->view.setParent(prevParent);
    surf()->requestNextFrame(false);
  } else {
    setExtraGeometry({0, TOPLEVEL_TOPBAR_HEIGHT, 0, 0});
    configureSize(pendingConfiguration().size -
                  LSize(0, TOPLEVEL_TOPBAR_HEIGHT));
    decoratedView = std::make_unique<ToplevelView>(this);
    decoratedView->updateGeometry();
    decoratedView->setVisible(surf()->view.visible());
    surf()->view.enableParentClipping(true);
    surf()->view.enableCustomPos(true);
    surf()->view.enableParentOffset(true);
  }

  if (!fullscreen() && rolePos().y() < TOPBAR_HEIGHT)
    surface()->setY(TOPBAR_HEIGHT);
}

void Toplevel::activatedChanged() {
  if (decoratedView) decoratedView->updateGeometry();

  if (activated()) {
    unsetMinimizedRequest();
    seat()->keyboard()->setFocus(surface());
    surface()->raise();

    if (!fullscreen() && !surf()->parent()) {
      Output *o = (Output *)cursor()->output();

      if (o && o->currentWorkspace != o->workspaces.front())
        o->setWorkspace(o->workspaces.front(), 600.f, 4.f);
    } else if (fullscreenWorkspace &&
               !fullscreenWorkspace->output->workspacesAnimation.running())
      fullscreenWorkspace->output->setWorkspace(fullscreenWorkspace, 600.f,
                                                4.f);
  }
}

void Toplevel::titleChanged() {
  if (decoratedView) decoratedView->updateTitle();
}

void Toplevel::unsetFullscreen() {
  if (surf()) surf()->view.enableCustomTranslucentRegion(false);

  if (fullscreenWorkspace && decoratedView) {
    decoratedView->decoT.insertAfter(&decoratedView->decoTL);
    decoratedView->topbarInput.insertAfter(&decoratedView->decoT);
    decoratedView->buttonsContainer.insertAfter(&decoratedView->topbarInput);
  }

  if (!fullscreenOutput) return;

  if (destructorCalled || quickUnfullscreen) {
    if (fullscreenWorkspace) {
      Workspace *prev = *std::prev(fullscreenWorkspace->outputLink);
      fullscreenOutput->workspacesAnimation.stop();
      fullscreenOutput->animatedFullscreenToplevel = nullptr;
      delete fullscreenWorkspace;
      fullscreenWorkspace = nullptr;
      fullscreenOutput->setWorkspace(prev, 600.f, 4.0f, 0.5f);
      fullscreenOutput = nullptr;
    }
    return;
  }

  if (decoratedView) decoratedView->updateGeometry();

  animScene = std::make_unique<LSceneView>(fullscreenOutput->sizeB(),
                                           fullscreenOutput->scale());
  animScene->setPos(fullscreenOutput->pos());
  G::reparentWithSubsurfaces(surf(), animScene.get(), true);
  fullscreenOutput->animatedFullscreenToplevel = this;

  captureView.enableDstSize(true);
  captureView.setDstSize(fullscreenOutput->size());
  captureView.setOpacity(1.f);
  captureView.setVisible(true);
  captureView.enableParentOpacity(false);

  animView.setParent(&G::compositor()->overlayLayer);
  captureView.insertAfter(&animView);

  animView.setTranslucentRegion(nullptr);
  animView.setOpacity(1.f);

  fullscreenOutput->setWorkspace(fullscreenOutput->workspaces.front(),
                                 WORKSPACE_ANIM_MS, 2.f);
  G::scene()->mainView()->damageAll(fullscreenOutput);
}

void Toplevel::preferredDecorationModeChanged() {
  // configureDecorationMode(preferredDecorationMode());
}
