#ifndef LSCREENSHOTREQUEST_H
#define LSCREENSHOTREQUEST_H

#include <LNamespaces.h>

/**
 * @brief Client request to take a screenshot.
 *
 * Clients using the [Wlr Screencopy](https://wayland.app/protocols/wlr-screencopy-unstable-v1) protocol can request
 * to take a screenshot of a specific region of an LOutput.
 *
 * The requests are only for the result of the current LOutput::paintGL() event and should only be handled within that event.\n
 * Use LOutput::screenshotRequests() to access the current requests and accept() to allow/deny the capture.
 *
 * @note By default, all captures are denied unless accept(true) is called within a LOutput::paintGL() event.
 */
class Louvre::LScreenshotRequest
{
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
     * @param accept Boolean value indicating whether to allow the capture (`true`) or deny it (`false`).
     *        It can be called multiple times with different responses,
     *        but only the last one before `LOutput::paintGL()` finishes is considered.
     *        If never called within `LOutput::paintGL()`, the capture is denied.
     * @note Only call this method within a `LOutput::paintGL()` event.
     */
    void accept(bool accept) noexcept;

    /**
     * @brief Wayland resource associated with the screenshot request.
     */
    Protocols::ScreenCopy::RScreenCopyFrame &resource() const noexcept { return m_screenCopyFrameRes; }
private:
    /// @cond OMIT
    friend class Protocols::ScreenCopy::RScreenCopyFrame;
    friend class LOutput;
    LScreenshotRequest(Protocols::ScreenCopy::RScreenCopyFrame &screenCopyFrameRes) noexcept : m_screenCopyFrameRes(screenCopyFrameRes) {};
    ~LScreenshotRequest() noexcept;
    Int8 copy() noexcept;
    Protocols::ScreenCopy::RScreenCopyFrame &m_screenCopyFrameRes;
    /// @endcond
};

#endif // LSCREENSHOTREQUEST_H
