#include "Topbar.h"

#include <LAnimation.h>
#include <LOutputMode.h>
#include <LSessionLockManager.h>
#include <LTextureView.h>

#include "../../common/TextRenderer.h"
#include "Compositor.h"
#include "Global.h"
#include "LCursor.h"
#include "Output.h"
#include "Pointer.h"

Topbar::Topbar(Output *output)
    : LSolidColorView(1.f, 1.f, 1.f, 0.82f, nullptr), output(output) {
  exclusiveZone.setOnRectChangeCallback([this](auto) { update(); });

  enableParentOffset(false);
  enablePointerEvents(true);
  enableBlockPointer(false);

  logo.enableCustomColor(true);
  logo.setCustomColor({0.1f, 0.1f, 0.1f});
  logo.setPos(12, 5);

  clock.enablePointerEvents(false);
  clock.setBufferScale(2);

  outputInfo.enablePointerEvents(false);
  outputInfo.setBufferScale(2);

  oversamplingLabel.enableCustomColor(true);
  oversamplingLabel.enablePointerEvents(false);
  oversamplingLabel.setBufferScale(2);

  vSyncLabel.enableCustomColor(true);
  vSyncLabel.enablePointerEvents(false);
  vSyncLabel.setBufferScale(2);

  appName.setBufferScale(2);
}

void Topbar::initialize() noexcept {
  logo.setTextureIndex(G::Logo);
  clock.setTexture(G::compositor()->clockTexture);
  oversamplingLabel.setTexture(G::compositor()->oversamplingLabelTexture);
  vSyncLabel.setTexture(G::compositor()->vSyncLabelTexture);
  appName.setTexture(G::textures()->defaultTopbarAppName);
  setParent(&G::compositor()->overlayLayer);
  exclusiveZone.setOutput(output);
  updateOutputInfo();
  update();

  for (LView *child : children()) child->enableParentOpacity(false);
}

void Topbar::update() {
  if (output->scale() != output->fractionalScale() &&
      output->fractionalOversamplingEnabled())
    oversamplingLabel.setCustomColor({0.1f, 0.8f, 0.1f});
  else
    oversamplingLabel.setCustomColor({0.8f, 0.1f, 0.1f});

  if (output->vSyncEnabled())
    vSyncLabel.setCustomColor({0.1f, 0.8f, 0.1f});
  else
    vSyncLabel.setCustomColor({0.8f, 0.1f, 0.1f});

  setVisible(true);
  setPos(output->pos() + exclusiveZone.rect().pos());
  setSize(exclusiveZone.rect().size());
  appName.setPos(42, 1 + (size().h() - appName.size().h()) / 2);
  clock.setPos(size().w() - clock.size().w() - 8,
               1 + (size().h() - clock.size().h()) / 2);
  vSyncLabel.setPos(clock.nativePos().x() - vSyncLabel.size().w() - 8,
                    clock.nativePos().y());
  oversamplingLabel.setPos(
      vSyncLabel.nativePos().x() - oversamplingLabel.size().w() - 8,
      vSyncLabel.nativePos().y());
  outputInfo.setPos(
      oversamplingLabel.nativePos().x() - outputInfo.size().w() - 8,
      oversamplingLabel.nativePos().y());

  const Int32 appNameX2{appName.pos().x() + appName.size().w()};
  outputInfo.setVisible(outputInfo.pos().x() > appNameX2);
  oversamplingLabel.setVisible(oversamplingLabel.pos().x() > appNameX2);
  vSyncLabel.setVisible(vSyncLabel.pos().x() > appNameX2);
  clock.setVisible(clock.pos().x() > appNameX2);
}

void Topbar::updateOutputInfo() {
  char info[256];

  sprintf(info, "%s  %dx%d @ %.1fHz @ %.2fx @ %s", output->name(),
          output->currentMode()->sizeB().w(),
          output->currentMode()->sizeB().h(),
          Float32(output->currentMode()->refreshRate()) / 1000,
          output->fractionalScale(), G::transformName(output->transform()));

  if (outputInfo.texture()) delete outputInfo.texture();

  outputInfo.setTexture(G::font()->regular->renderText(info, 22));
}

void Topbar::uninitialize() noexcept {
  setParent(nullptr);
  exclusiveZone.setOutput(nullptr);
}

void Topbar::pointerEnterEvent(const LPointerEnterEvent &) {
  if (seat()->toplevelResizeSessions().empty() &&
      seat()->toplevelMoveSessions().empty() && !G::pointer()->cursorOwner)
    cursor()->useDefault();
}

void Topbar::pointerMoveEvent(const LPointerMoveEvent &) {
  if (seat()->toplevelResizeSessions().empty() &&
      seat()->toplevelMoveSessions().empty() && !G::pointer()->cursorOwner)
    cursor()->useDefault();
}

bool Topbar::nativeMapped() const noexcept {
  return compositor()->sessionLockManager()->state() ==
         LSessionLockManager::Unlocked;
}
