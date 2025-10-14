#ifndef LCLIENTPRIVATE_H
#define LCLIENTPRIVATE_H

#include <CZ/Louvre/LClient.h>

using namespace CZ;
using namespace CZ::Protocols;

struct LClient::Params
{
    wl_client *client;
};

class LClient::LClientPrivate
{
public:
    LClientPrivate(LClient *, wl_client *wlClient) noexcept :
        client {wlClient}
    {}

    ~LClientPrivate() noexcept = default;

    wl_client *client;
    EventHistory eventHistory;
    std::shared_ptr<LCursorSource> cursor;
    std::list<LResource*> resources;

    // Globals
    std::vector<Wayland::GSeat*> seatGlobals;
    std::vector<Wayland::GOutput*> outputGlobals;
    std::vector<Wayland::GDataDeviceManager*> dataDeviceManagerGlobals;
    std::vector<Wayland::GCompositor*> compositorGlobals;
    std::vector<Wayland::GSubcompositor*> subcompositorGlobals;
    std::vector<XdgShell::GXdgWmBase*> xdgWmBaseGlobals;
    std::vector<XdgDecoration::GXdgDecorationManager*> xdgDecorationManagerGlobals;
    std::vector<XdgOutput::GXdgOutputManager*> xdgOutputManagerGlobals;
    std::vector<PresentationTime::GPresentation*> presentationTimeGlobals;
    std::vector<LinuxDMABuf::GZwpLinuxDmaBufV1*> linuxDMABufGlobals;
    std::vector<Viewporter::GViewporter*> viewporterGlobals;
    std::vector<FractionalScale::GFractionalScaleManager*> fractionalScaleManagerGlobals;
    std::vector<GammaControl::GZwlrGammaControlManagerV1*> gammaControlManagerGlobals;
    std::vector<TearingControl::GTearingControlManager*> tearingControlManagerGlobals;
    std::vector<RelativePointer::GRelativePointerManager*> relativePointerManagerGlobals;
    std::vector<PointerGestures::GPointerGestures*> pointerGesturesGlobals;
    std::vector<SessionLock::GSessionLockManager*> sessionLockManagerGlobals;
    std::vector<PointerConstraints::GPointerConstraints*> pointerConstraintsGlobals;
    std::vector<LayerShell::GLayerShell*> layerShellGlobals;
    std::vector<ForeignToplevelManagement::GForeignToplevelManager*> foreignToplevelManagerGlobals;
    std::vector<ForeignToplevelList::GForeignToplevelList*> foreignToplevelListGlobals;
    std::vector<SinglePixelBuffer::GWpSinglePixelBufferManagerV1*> singlePixelBufferManagerGlobals;
    std::vector<ContentType::GWpContentTypeManagerV1*> contentTypeManagerGlobals;
    std::vector<IdleNotify::GIdleNotifier*> idleNotifierGlobals;
    std::vector<IdleInhibit::GIdleInhibitManager*> idleInhibitManagerGlobals;
    std::vector<XdgActivation::GXdgActivation*> xdgActivationGlobals;
    std::vector<DRMLease::GDRMLeaseDevice*> drmLeaseDeviceGlobals;
    std::vector<ImageCaptureSource::GOutputImageCaptureSourceManager*> outputImageCaptureSourceManagerGlobals;
    std::vector<ImageCaptureSource::GForeignToplevelImageCaptureSourceManager*> foreignToplevelImageCaptureSourceManagerGlobals;
    std::vector<WlrOutputManagement::GWlrOutputManager*> wlrOutputManagerGlobals;
    std::vector<BackgroundBlur::GBackgroundBlurManager*> backgroundBlurManagerGlobals;
    std::vector<CursorShape::GCursorShapeManager*> cursorShapeManagerGlobals;
    std::vector<SvgPath::GSvgPathManager*> svgPathManagerGlobals;
    std::vector<InvisibleRegion::GInvisibleRegionManager*> invisibleRegionManagerGlobals;
    std::vector<WaylandDRM::GWlDRM*> wlDRMGlobals;
    std::vector<DRMSyncObj::GDRMSyncObjManager*> drmSyncObjManagerGlobals;

    bool pendingDestroyLater { false };
    bool destroyed { false };
};

#endif // LCLIENTPRIVATE_H
