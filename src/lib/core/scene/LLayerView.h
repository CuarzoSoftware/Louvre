#ifndef LLAYERVIEW_H
#define LLAYERVIEW_H

#include <LView.h>

/**
 * @brief Container of views
 *
 * The LLayerView is a non-renderable LView. Unlike other views, it doesn't have
 * content to render on its own.\n Instead, it functions as a container for
 * other views, allowing you to stack a group of views together or apply
 * clipping to them.
 */
class Louvre::LLayerView : public LView {
 public:
  /**
   * @brief Constructor for LLayerView.
   *
   * @param parent The parent view, if any.
   */
  LLayerView(LView *parent = nullptr) noexcept
      : LView(LView::LayerType, false, parent) {}

  LCLASS_NO_COPY(LLayerView)

  /**
   * @brief Destructor for LLayerView.
   */
  ~LLayerView() noexcept { notifyDestruction(); };

  /**
   * @brief Sets the position of the view.
   *
   * @param x The x-coordinate in surface coordinates.
   * @param y The y-coordinate in surface coordinates.
   */
  void setPos(Int32 x, Int32 y) noexcept {
    if (x == m_nativePos.x() && y == m_nativePos.y()) return;

    m_nativePos.setX(x);
    m_nativePos.setY(y);

    if (!repaintCalled() && mapped()) repaint();
  }

  /**
   * @brief Sets the position of the view.
   *
   * @param pos The position in surface coordinates.
   */
  void setPos(const LPoint &pos) noexcept { setPos(pos.x(), pos.y()); }

  /**
   * @brief Sets the size of the view.
   *
   * @param size The size in surface coordinates.
   */
  void setSize(const LSize &size) noexcept { setSize(size.w(), size.h()); }

  /**
   * @brief Sets the size of the view.
   *
   * @param width The width in surface coordinates.
   * @param height The height in surface coordinates.
   */
  void setSize(Int32 width, Int32 height) noexcept {
    if (width == m_nativeSize.w() && height == m_nativeSize.h()) return;

    m_nativeSize.setW(width);
    m_nativeSize.setH(height);

    if (!repaintCalled() && mapped()) repaint();
  }

  /**
   * @brief Sets the input region of the view.
   *
   * @param region The input region to be set or `nullptr` to allow the entire
   * view to receive events.
   */
  void setInputRegion(const LRegion *region) noexcept {
    if (region) {
      if (m_inputRegion)
        *m_inputRegion = *region;
      else
        m_inputRegion = std::make_unique<LRegion>(*region);
    } else
      m_inputRegion.reset();
  }

  virtual bool nativeMapped() const noexcept override;
  virtual const LPoint &nativePos() const noexcept override;
  virtual const LSize &nativeSize() const noexcept override;
  virtual Float32 bufferScale() const noexcept override;
  virtual void enteredOutput(LOutput *output) noexcept override;
  virtual void leftOutput(LOutput *output) noexcept override;
  virtual const std::vector<LOutput *> &outputs() const noexcept override;
  virtual void requestNextFrame(LOutput *output) noexcept override;
  virtual const LRegion *damage() const noexcept override;
  virtual const LRegion *translucentRegion() const noexcept override;
  virtual const LRegion *opaqueRegion() const noexcept override;
  virtual const LRegion *inputRegion() const noexcept override;
  virtual void paintEvent(const PaintEventParams &params) noexcept override;

 protected:
  std::vector<LOutput *> m_outputs;
  std::unique_ptr<LRegion> m_inputRegion;
  LPoint m_nativePos;
  LSize m_nativeSize{256, 256};
};

#endif  // LLAYERVIEW_H
