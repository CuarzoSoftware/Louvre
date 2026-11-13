#ifndef LIMAGECAPTUREMANAGER_H
#define LIMAGECAPTUREMANAGER_H

#include <CZ/Louvre/LFactoryObject.h>
#include <vector>

/**
 * @brief Image capture sessions (screenshots, screen recording and sharing)
 *
 * Clients capture outputs and toplevels through the
 * [ext-image-copy-capture](https://wayland.app/protocols/ext-image-copy-capture-v1) protocol, choosing
 * what to capture with [ext-image-capture-source](https://wayland.app/protocols/ext-image-capture-source-v1).
 * Each capture is an LImageCaptureSession, which delivers frames (LImageCaptureFrame) to the client.
 *
 * Capturing is a privacy matter, so every new session must be accepted first: sessionRequest() is
 * called and the session stays pending, without producing frames, until LImageCaptureSession::accept()
 * or LImageCaptureSession::stop() is called. This can happen at any later time, e.g. after asking the
 * user with a modal dialog. The default implementation accepts every session right away.
 *
 * Once accepted, everything is handled by Louvre:
 * - **Outputs**: frames are copied from the output images after each paintGL(), only when something
 *   changed and only the changed regions (with or without the cursor, as the client asks).
 * - **Toplevels**: frames are rendered by LToplevelRole::imageCaptureFrameRequest() when its surfaces
 *   are damaged. The default implementation draws the toplevel's surface and subsurfaces; override
 *   it to e.g. include server-side decorations.
 *
 * There is a unique instance of this class, created within LCompositor::createObjectRequest(),
 * which can be accessed globally via CZ::imageCaptureManager().
 *
 * To disable these protocols, remove their globals from LCompositor::createGlobalsRequest().
 *
 * @see LImageCaptureSession and LImageCaptureFrame
 */
class CZ::LImageCaptureManager : public LFactoryObject
{
public:
    static constexpr LFactoryObject::Type FactoryObjectType = LFactoryObject::Type::LImageCaptureManager;

    /**
     * @brief LImageCaptureManager class constructor.
     *
     * There is only one instance of this class, which can be accessed globally via imageCaptureManager().
     *
     * @param params Internal parameters provided in LCompositor::createObjectRequest().
     */
    LImageCaptureManager(const void *params) noexcept;

    /**
     * @brief Destructor of the LImageCaptureManager class.
     *
     * Invoked after LCompositor::onAnticipatedObjectDestruction().
     */
    ~LImageCaptureManager() noexcept;

    /**
     * @brief Every capture session, pending, accepted or stopped (in creation order).
     *
     * E.g. to show the user that the screen is being captured: accepted sessions.
     */
    const std::vector<LImageCaptureSession*> &sessions() const noexcept { return m_sessions; }

    /**
     * @brief A client wants to start capturing.
     *
     * The session is pending: no frames are captured until LImageCaptureSession::accept() is called,
     * now or later (e.g. after asking the user). Call LImageCaptureSession::stop() to deny it.
     *
     * #### Default implementation
     * @snippet LImageCaptureManagerDefault.cpp sessionRequest
     */
    virtual void sessionRequest(LImageCaptureSession *session);

    /**
     * @brief A session is about to be destroyed.
     *
     * Destroyed by its client (or when it disconnects). It's no longer in sessions() and must not be
     * used after this method returns (e.g. close a dialog asking about it).
     *
     * #### Default implementation
     * @snippet LImageCaptureManagerDefault.cpp sessionDestroyed
     */
    virtual void sessionDestroyed(LImageCaptureSession *session);

private:
    friend class LImageCaptureSession;
    friend class Protocols::ImageCopyCapture::GImageCopyCaptureManager;
    friend class Protocols::ImageCopyCapture::RImageCopyCaptureCursorSession;
    std::vector<LImageCaptureSession*> m_sessions;
};

#endif // LIMAGECAPTUREMANAGER_H
