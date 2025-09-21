#include <LPainter.h>
#include <LSolidColorView.h>
#include <LUtils.h>

using namespace Louvre;

bool LSolidColorView::nativeMapped() const noexcept { return true; }

const LPoint &LSolidColorView::nativePos() const noexcept {
  return m_nativePos;
}

const LSize &LSolidColorView::nativeSize() const noexcept {
  return m_nativeSize;
}

Float32 LSolidColorView::bufferScale() const noexcept { return 1.f; }

void LSolidColorView::enteredOutput(LOutput *output) noexcept {
  LVectorPushBackIfNonexistent(m_outputs, output);
}

void LSolidColorView::leftOutput(LOutput *output) noexcept {
  LVectorRemoveOneUnordered(m_outputs, output);
}

const std::vector<LOutput *> &LSolidColorView::outputs() const noexcept {
  return m_outputs;
}

void LSolidColorView::requestNextFrame(LOutput *output) noexcept {
  L_UNUSED(output);
}

const LRegion *LSolidColorView::damage() const noexcept {
  return &LRegion::EmptyRegion();
}

const LRegion *LSolidColorView::translucentRegion() const noexcept {
  if (m_opacity < 1.f) return nullptr;

  return &LRegion::EmptyRegion();
}

const LRegion *LSolidColorView::opaqueRegion() const noexcept {
  return nullptr;
}

const LRegion *LSolidColorView::inputRegion() const noexcept {
  return m_inputRegion.get();
}

void LSolidColorView::paintEvent(const PaintEventParams &params) noexcept {
  params.painter->setColor(color());
  params.painter->bindColorMode();
  params.painter->drawRegion(*params.region);
}
