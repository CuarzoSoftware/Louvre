#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <LRegion.h>
#include <LSolidColorView.h>
#include <LTextureView.h>
#include <LToplevelRole.h>

class ToplevelView;
class Output;
class Workspace;

using namespace Louvre;

class Toplevel final : public LToplevelRole {
 public:
  Toplevel(const void *params);
  ~Toplevel();

  // Quick parse handles
  class Surface *surf() const { return (Surface *)surface(); }

  const LPoint &rolePos() const override;
  void configureRequest() override;
  void atomsChanged(LBitset<AtomChanges> changes, const Atoms &prev) override;
  void startResizeRequest(const LEvent &triggeringEvent,
                          LBitset<LEdge> edge) override;
  void setMaximizedRequest() override;
  void unsetMaximizedRequest() override;
  void maximizedChanged();
  void activatedChanged();
  void setFullscreenRequest(LOutput *output) override;
  void unsetFullscreenRequest() override;
  void fullscreenChanged();
  void setMinimizedRequest() override;
  void unsetMinimizedRequest() override;
  void preferredDecorationModeChanged() override;
  void decorationModeChanged();
  void titleChanged() override;
  void unsetFullscreen();

  bool requestedFullscreenOnFirstMap{false};
  bool destructorCalled{false};
  bool quickUnfullscreen{false};

  std::unique_ptr<ToplevelView> decoratedView;

  LSolidColorView blackFullscreenBackground;

  // Rendered view for fullscreen animation
  LTextureView captureView;
  std::unique_ptr<LTexture> captureTexture;
  LRegion captureTransRegion;

  // Rects for fullscreen animation
  LRect dstRect, prevBoundingRect;
  LWeak<Output> fullscreenOutput;
  LWeak<Workspace> fullscreenWorkspace;
  UInt32 prevStates{0};
  UInt32 outputUnplugConfigureCount{0};

  LTextureView animView;
  std::unique_ptr<LSceneView> animScene;
};

#endif  // TOPLEVEL_H
