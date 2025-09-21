#ifndef LSCREENSHOTREQUEST_H
#define LSCREENSHOTREQUEST_H

#include <LNamespaces.h>

/**
 * @brief Request to capture an LOutput frame
 *
 * @anchor lscreenshotrequest_detailed
 *
 * Clients using the [Wlr
 * Screencopy](https://wayland.app/protocols/wlr-screencopy-unstable-v1)
 * protocol can request to capture a specific region of an LOutput.
 *
 * The LScreenshotRequest class represents a single frame wanted to be captured,
 * and must be handled within an LOutput::paintGL() event.\n This means that for
 * screencasting, clients make a new LScreenshotRequest for each paintGL
 * event.\n If a request is accepted within a paintGL event, Louvre later copies
 * the rendered frame to the client's buffer.
 *
 * @note All requests are initially denied unless accept(true) is called and no
 * custom buffer is set for direct scanout (see
 * LOutput::setCustomScanoutBuffer()).
 *
 * Within a paintGL event, the LOutput::screenshotRequests() vector contains the
 * requests made for the current frame being rendered, which can be more than
 * one if multiple clients are requesting to capture it.
 *
 * @note It's recommended to use LCompositor::globalsFilter() to permit only a
 * single well-known client to use this protocol, such as the
 * `xdg-desktop-portal-wlr`.
 */
class Louvre::LScreenshotRequest {
 public:
  /**
   * @brief Client requesting to take the screenshot.
   */
  LClient *client() const noexcept;

  /**
   * @brief Rect within the output requested to be captured.
   *
   * Coordinates are in surface coordinates relative to the output's position.
   */
  const LRect &rect() const noexcept;

  /**
   * @brief Respond to the screenshot request.
   * @param accept Boolean value indicating whether to allow the capture
   * (`true`) or deny it (`false`). It can be called multiple times with
   * different responses, but only the last one before `LOutput::paintGL()`
   * finishes is considered. If never called within `LOutput::paintGL()`, the
   * capture is denied.
   * @note Only call this method within a `LOutput::paintGL()` event.
   */
  void accept(bool accept) noexcept;

  /**
   * @brief Wayland resource associated with the screenshot request.
   */
  Protocols::ScreenCopy::RScreenCopyFrame &resource() const noexcept {
    return m_screenCopyFrameRes;
  }

 private:
  friend class Protocols::ScreenCopy::RScreenCopyFrame;
  friend class LOutput;
  LScreenshotRequest(
      Protocols::ScreenCopy::RScreenCopyFrame &screenCopyFrameRes) noexcept
      : m_screenCopyFrameRes(screenCopyFrameRes) {};
  ~LScreenshotRequest() noexcept = default;
  Int8 copy() noexcept;
  Protocols::ScreenCopy::RScreenCopyFrame &m_screenCopyFrameRes;
};

#endif  // LSCREENSHOTREQUEST_H
