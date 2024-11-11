#ifndef LCLIENTPRIVATE_H
#define LCLIENTPRIVATE_H

#include <LClient.h>
#include <LClientCursor.h>

using namespace Louvre;
using namespace Louvre::Protocols;

struct LClient::Params
{
    wl_client *client;
};

class LClient::LClientPrivate
{
public:
    LClientPrivate(LClient *lClient, wl_client *wlClient) noexcept :
        client {wlClient},
        lastCursorRequest {lClient}
    {}

    LCLASS_NO_COPY(LClientPrivate)

    ~LClientPrivate() noexcept = default;

    wl_client *client;
    EventHistory eventHistory;
    LClientCursor lastCursorRequest;

    // Globals
    std::vector<Wayland::GSeat*> seatGlobals;
    std::vector<Wayland::GOutput*> outputGlobals;
    std::vector<ScreenCopy::GScreenCopyManager*> screenCopyManagerGlobals;
    std::vector<Wayland::GDataDeviceManager*> dataDeviceManagerGlobals;
    std::vector<Wayland::GCompositor*> compositorGlobals;
    std::vector<Wayland::GSubcompositor*> subcompositorGlobals;
    std::vector<XdgShell::GXdgWmBase*> xdgWmBaseGlobals;
    std::vector<XdgDecoration::GXdgDecorationManager*> xdgDecorationManagerGlobals;
    std::vector<XdgOutput::GXdgOutputManager*> xdgOutputManagerGlobals;
    std::vector<PresentationTime::GPresentation*> presentationTimeGlobals;
    std::vector<LinuxDMABuf::GLinuxDMABuf*> linuxDMABufGlobals;
    std::vector<Viewporter::GViewporter*> viewporterGlobals;
    std::vector<FractionalScale::GFractionalScaleManager*> fractionalScaleManagerGlobals;
    std::vector<GammaControl::GGammaControlManager*> gammaControlManagerGlobals;
    std::vector<TearingControl::GTearingControlManager*> tearingControlManagerGlobals;
    std::vector<RelativePointer::GRelativePointerManager*> relativePointerManagerGlobals;
    std::vector<PointerGestures::GPointerGestures*> pointerGesturesGlobals;
    std::vector<SessionLock::GSessionLockManager*> sessionLockManagerGlobals;
    std::vector<PointerConstraints::GPointerConstraints*> pointerConstraintsGlobals;
    std::vector<LayerShell::GLayerShell*> layerShellGlobals;
    std::vector<ForeignToplevelManagement::GForeignToplevelManager*> foreignToplevelManagerGlobals;
    std::vector<ForeignToplevelList::GForeignToplevelList*> foreignToplevelListGlobals;
    std::vector<SinglePixelBuffer::GSinglePixelBufferManager*> singlePixelBufferManagerGlobals;
    std::vector<ContentType::GContentTypeManager*> contentTypeManagerGlobals;
    std::vector<IdleNotify::GIdleNotifier*> idleNotifierGlobals;
    std::vector<IdleInhibit::GIdleInhibitManager*> idleInhibitManagerGlobals;
    std::vector<XdgActivation::GXdgActivation*> xdgActivationGlobals;
    std::vector<DRMLease::GDRMLeaseDevice*> drmLeaseDeviceGlobals;
    std::vector<ImageCaptureSource::GOutputImageCaptureSourceManager*> outputImageCaptureSourceManagerGlobals;
    std::vector<ImageCaptureSource::GForeignToplevelImageCaptureSourceManager*> foreignToplevelImageCaptureSourceManagerGlobals;
    std::vector<WlrOutputManagement::GWlrOutputManager*> wlrOutputManagerGlobals;
    bool destroyed { false };
};

#endif // LCLIENTPRIVATE_H
