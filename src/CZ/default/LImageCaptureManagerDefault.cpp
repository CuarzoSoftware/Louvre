#include <CZ/Louvre/Manager/LImageCaptureManager.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>

using namespace CZ;

//! [sessionRequest]
void LImageCaptureManager::sessionRequest(LImageCaptureSession *session)
{
    /* Accept every session right away. To ask the user first, keep the session
     * (e.g. in a CZWeak) and call accept() or stop() once answered. */
    session->accept();
}
//! [sessionRequest]

//! [sessionDestroyed]
void LImageCaptureManager::sessionDestroyed(LImageCaptureSession *session)
{
    CZ_UNUSED(session)
}
//! [sessionDestroyed]
