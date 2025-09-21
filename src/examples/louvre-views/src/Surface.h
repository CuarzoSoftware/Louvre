#ifndef SURFACE_H
#define SURFACE_H

#include <LAnimation.h>
#include <LSurface.h>
#include <LSurfaceView.h>
#include <LTimer.h>

#include "DockItem.h"
#include "Global.h"

using namespace Louvre;
using namespace std;

class Toplevel;

class Surface : public LSurface {
 public:
  Surface(const void *params);
  ~Surface();

  class Toplevel *tl() const { return (class Toplevel *)toplevel(); }

  static class Surface *searchSessionLockParent(Surface *parent);
  class Toplevel *closestToplevelParent() const noexcept;

  LView *getView() const;

  void parentChanged() override;
  void mappingChanged() override;
  void orderChanged() override;
  void roleChanged(LBaseSurfaceRole *prevRole) override;
  void bufferSizeChanged() override;
  void minimizedChanged() override;
  void damageChanged() override;
  void preferVSyncChanged() override;

  void onToplevelFirstMap() noexcept;

  LTexture *renderThumbnail(LRegion *transRegion = nullptr);
  void unminimize(DockItem *clickedItem);

  LAnimation firstMapAnim;
  bool firstMap = true;
  LSurfaceView view;

  LTextureView *thumbnailFullsizeView = nullptr;
  LTexture *thumbnailFullSizeTex = nullptr;
  LTexture *thumbnailTex = nullptr;
  std::list<DockItem *> minimizedViews;

  LRect minimizeStartRect;
  LAnimation minimizeAnim;
  Output *minimizedOutput = nullptr;
  LRegion minimizedTransRegion;

  LPoint localOutputPos;
  LSize localOutputSize;
  bool outputUnplugHandled = true;

  bool fadedOut = false;

  UInt32 prevDamageId;
  LTimer firstMapTimer{[this](auto) { onToplevelFirstMap(); }};
};

#endif  // SURFACE_H
