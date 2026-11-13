#include <CZ/Louvre/Protocols/ImageCopyCapture/ext-image-copy-capture-v1.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/GImageCopyCaptureManager.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureSession.h>
#include <CZ/Louvre/Protocols/ImageCopyCapture/RImageCopyCaptureCursorSession.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/RImageCaptureSource.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/RForeignToplevelHandle.h>
#include <CZ/Louvre/Protocols/Wayland/GOutput.h>
#include <CZ/Louvre/Manager/LImageCaptureManager.h>
#include <CZ/Louvre/Manager/LImageCaptureSession.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/LLog.h>

using namespace CZ;
using namespace CZ::Protocols::ImageCopyCapture;

static const struct ext_image_copy_capture_manager_v1_interface imp
{
    .create_session = &GImageCopyCaptureManager::create_session,
    .create_pointer_cursor_session = &GImageCopyCaptureManager::create_pointer_cursor_session,
    .destroy = &GImageCopyCaptureManager::destroy
};

LGLOBAL_INTERFACE_IMP(GImageCopyCaptureManager, LOUVRE_IMAGE_COPY_CAPTURE_MANAGER_VERSION, ext_image_copy_capture_manager_v1_interface)

bool GImageCopyCaptureManager::Probe(CZWeak<LGlobal> **slot) noexcept
{
    if (compositor()->wellKnownGlobals.ImageCopyCaptureManager)
    {
        LLog(CZError, CZLN, "Failed to create {} global (already created)", Interface()->name);
        return false;
    }

    *slot = &compositor()->wellKnownGlobals.ImageCopyCaptureManager;
    return true;
}

GImageCopyCaptureManager::GImageCopyCaptureManager
    (
        wl_client *client,
        Int32 version,
        UInt32 id
    ) noexcept
    :LResource
    (
        client,
        Interface(),
        version,
        id,
        &imp
    )
{}

/******************** REQUESTS ********************/

void GImageCopyCaptureManager::create_session(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *source, UInt32 options) noexcept
{
    auto &res { LRES_CAST(GImageCopyCaptureManager, resource) };

    if (options & ~UInt32(EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS))
    {
        res.postError(EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_ERROR_INVALID_OPTION, "Invalid options {}.", options);
        return;
    }

    auto &sourceRes { LRES_CAST(ImageCaptureSource::RImageCaptureSource, source) };
    auto *sessionRes { new RImageCopyCaptureSession(res.client(), res.version(), id) };

    // The source object (wl_output / foreign toplevel handle) and what it refers to
    LOutput *output {};
    LToplevelRole *toplevel {};

    if (sourceRes.type() == LImageCaptureSourceType::Output)
        output = sourceRes.output();
    else
        toplevel = sourceRes.toplevel();

    // Nothing to capture anymore
    if (!output && !toplevel)
    {
        sessionRes->stopped();
        return;
    }

    sessionRes->m_session.reset(new LImageCaptureSession(*sessionRes, sourceRes.type(), output, toplevel,
                                                         options & EXT_IMAGE_COPY_CAPTURE_MANAGER_V1_OPTIONS_PAINT_CURSORS, nullptr));
    auto *session { sessionRes->m_session.get() };
    imageCaptureManager()->m_sessions.emplace_back(session);

    // The compositor decides (possibly later) whether to accept it
    imageCaptureManager()->sessionRequest(session);
}

void GImageCopyCaptureManager::create_pointer_cursor_session(wl_client */*client*/, wl_resource *resource, UInt32 id, wl_resource *source, wl_resource */*pointer*/) noexcept
{
    // There's a single pointer (seat), whichever wl_pointer it is
    auto &res { LRES_CAST(GImageCopyCaptureManager, resource) };
    auto &sourceRes { LRES_CAST(ImageCaptureSource::RImageCaptureSource, source) };
    LOutput *output {};
    LToplevelRole *toplevel {};

    if (sourceRes.type() == LImageCaptureSourceType::Output)
        output = sourceRes.output();
    else
        toplevel = sourceRes.toplevel();

    new RImageCopyCaptureCursorSession(res.client(), res.version(), id, sourceRes.type(), output, toplevel);
}

void GImageCopyCaptureManager::destroy(wl_client */*client*/, wl_resource *resource) noexcept
{
    wl_resource_destroy(resource);
}
