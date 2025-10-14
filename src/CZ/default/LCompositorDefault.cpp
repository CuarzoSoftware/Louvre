#include <CZ/Louvre/Protocols/ImageCaptureSource/GForeignToplevelImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/GOutputImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ForeignToplevelManagement/GForeignToplevelManager.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/GWpSinglePixelBufferManagerV1.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/GForeignToplevelList.h>
#include <CZ/Louvre/Protocols/RelativePointer/GRelativePointerManager.h>
#include <CZ/Louvre/Protocols/FractionalScale/GFractionalScaleManager.h>
#include <CZ/Louvre/Protocols/InvisibleRegion/GInvisibleRegionManager.h>
#include <CZ/Louvre/Protocols/PointerConstraints/GPointerConstraints.h>
#include <CZ/Louvre/Protocols/TearingControl/GTearingControlManager.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/GBackgroundBlurManager.h>
#include <CZ/Louvre/Protocols/XdgDecoration/GXdgDecorationManager.h>
#include <CZ/Louvre/Protocols/GammaControl/GZwlrGammaControlManagerV1.h>
#include <CZ/Louvre/Protocols/PointerGestures/GPointerGestures.h>
#include <CZ/Louvre/Protocols/SessionLock/GSessionLockManager.h>
#include <CZ/Louvre/Protocols/ContentType/GWpContentTypeManagerV1.h>
#include <CZ/Louvre/Protocols/IdleInhibit/GIdleInhibitManager.h>
#include <CZ/Louvre/Protocols/CursorShape/GCursorShapeManager.h>
#include <CZ/Louvre/Protocols/PresentationTime/GPresentation.h>
#include <CZ/Louvre/Protocols/XdgActivation/GXdgActivation.h>
#include <CZ/Louvre/Protocols/XdgOutput/GXdgOutputManager.h>
#include <CZ/Louvre/Protocols/Wayland/GDataDeviceManager.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/GZwpLinuxDmaBufV1.h>
#include <CZ/Louvre/Protocols/DRMSyncObj/GDRMSyncObjManager.h>
#include <CZ/Louvre/Protocols/IdleNotify/GIdleNotifier.h>
#include <CZ/Louvre/Protocols/SvgPath/GSvgPathManager.h>
#include <CZ/Louvre/Protocols/Viewporter/GViewporter.h>
#include <CZ/Louvre/Protocols/LayerShell/GLayerShell.h>
#include <CZ/Louvre/Protocols/Wayland/GSubcompositor.h>
#include <CZ/Louvre/Protocols/XdgShell/GXdgWmBase.h>
#include <CZ/Louvre/Protocols/Wayland/GCompositor.h>
#include <CZ/Louvre/Protocols/WaylandDRM/GWlDRM.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <CZ/Louvre/Manager/LSessionLockManager.h>
#include <CZ/Louvre/Roles/LSessionLockRole.h>
#include <CZ/Louvre/LCompositor.h>
#include <CZ/Louvre/Roles/LToplevelRole.h>
#include <CZ/Louvre/Cursor/LCursor.h>
#include <CZ/Louvre/Cursor/LCursorSource.h>
#include <CZ/Louvre/Roles/LSubsurfaceRole.h>
#include <CZ/Louvre/Seat/LPointer.h>
#include <CZ/Louvre/Seat/LKeyboard.h>
#include <CZ/Louvre/Seat/LTouch.h>
#include <CZ/Louvre/Roles/LSurface.h>
#include <CZ/Louvre/Seat/LDND.h>
#include <CZ/Louvre/Seat/LClipboard.h>
#include <CZ/Louvre/Seat/LOutput.h>
#include <CZ/Louvre/Seat/LSeat.h>
#include <CZ/Louvre/Roles/LPopupRole.h>
#include <CZ/Louvre/Roles/LCursorRole.h>
#include <CZ/Louvre/LLog.h>
#include <CZ/Louvre/LClient.h>
#include <CZ/Louvre/Roles/LDNDIconRole.h>
#include <CZ/Louvre/LGlobal.h>
#include <CZ/Louvre/Backends/LBackend.h>
#include <CZ/Ream/RCore.h>

using namespace CZ;
using namespace CZ::Protocols;

//! [createGlobalsRequest]
bool LCompositor::createGlobalsRequest()
{
    // Adds explicit sync support
    createGlobal<DRMSyncObj::GDRMSyncObjManager>();

    // Allows clients to create surfaces and regions
    createGlobal<Wayland::GCompositor>();

    // Allows clients to receive pointer, keyboard, and touch events
    createGlobal<Wayland::GSeat>();

    // Provides detailed information of pointer movement
    createGlobal<RelativePointer::GRelativePointerManager>();

    // Allows clients to request setting pointer constraints
    createGlobal<PointerConstraints::GPointerConstraints>();

    // Allows clients to receive swipe, pinch, and hold pointer gestures
    createGlobal<PointerGestures::GPointerGestures>();

    // Enables drag & drop and clipboard data sharing between clients
    createGlobal<Wayland::GDataDeviceManager>();

    // Allows clients to create subsurface roles
    createGlobal<Wayland::GSubcompositor>();

    // Allows clients to create toplevel and popup roles
    createGlobal<XdgShell::GXdgWmBase>();

    // Allows clients to request modifying the state of foreign toplevels
    createGlobal<ForeignToplevelManagement::GForeignToplevelManager>();

    // Allows clients to get handles of foreign toplevels
    createGlobal<ForeignToplevelList::GForeignToplevelList>();

    // Provides additional info about outputs
    createGlobal<XdgOutput::GXdgOutputManager>();

    // Allow negotiation of server-side or client-side decorations
    createGlobal<XdgDecoration::GXdgDecorationManager>();

    // Allow clients to adjust their surfaces buffers to fractional scales
    createGlobal<FractionalScale::GFractionalScaleManager>();

    // Allow clients to request setting the gamma LUT of outputs
    createGlobal<GammaControl::GZwlrGammaControlManagerV1>();

    // Allow clients to create DMA buffers
    createGlobal<WaylandDRM::GWlDRM>();
    createGlobal<LinuxDMABuf::GZwpLinuxDmaBufV1>();

    // Provides detailed information of how surfaces are presented
    createGlobal<PresentationTime::GPresentation>();

    // Allows clients to request locking the user session with arbitrary graphics
    createGlobal<SessionLock::GSessionLockManager>();

    // Allows clients to notify their preference of vsync for specific surfaces
    createGlobal<TearingControl::GTearingControlManager>();

    // Allows clients to clip and scale buffers
    createGlobal<Viewporter::GViewporter>();

    // Allows toplevels to be used as image capture sources
    //createGlobal<ImageCaptureSource::GForeignToplevelImageCaptureSourceManager>();

    // Allows outputs to be used as image capture sources
    //createGlobal<ImageCaptureSource::GOutputImageCaptureSourceManager>();

    // Allows clients to create wlr_layer_shell surfaces
    createGlobal<LayerShell::GLayerShell>();

    // Allows clients to create single pixel buffers (requires Viewporter::GViewporter)
    createGlobal<SinglePixelBuffer::GWpSinglePixelBufferManagerV1>();

    // Allows clients to provide a hint about the content type being displayed by surfaces
    createGlobal<ContentType::GWpContentTypeManagerV1>();

    // Notifies clients if the user has been idle for a given amount of time
    createGlobal<IdleNotify::GIdleNotifier>();

    // Allows clients to request inhibition of the compositor's idle state
    createGlobal<IdleInhibit::GIdleInhibitManager>();

    // Allows clients to activate other client's surfaces
    createGlobal<XdgActivation::GXdgActivation>();

    // Allows clients to arrange/set output properties
    createGlobal<WlrOutputManagement::GWlrOutputManager>();

    // Allows clients to set the cursor using pre defined shapes
    createGlobal<CursorShape::GCursorShapeManager>();

    // Allows clients to set background blur effects for surfaces
    createGlobal<BackgroundBlur::GBackgroundBlurManager>();

    // Allows clients to share SVG paths
    createGlobal<SvgPath::GSvgPathManager>();

    // Allows clients to specify invisible regions within their surfaces
    createGlobal<InvisibleRegion::GInvisibleRegionManager>();

    return true;
}
//! [createGlobalsRequest]

//! [globalsFilter]
bool LCompositor::globalsFilter(LClient *client, LGlobal *global)
{
    CZ_UNUSED(client)
    CZ_UNUSED(global)
    return true;
}
//! [globalsFilter]

//! [onPosixSignal]
void LCompositor::onPosixSignal(int signal)
{
    LLog(CZDebug, CZLN, "Posix Signal {} handled", signal);
}
//! [onPosixSignal]

//! [initialized]
void LCompositor::initialized() noexcept
{
    loadCursorShapes();

    SkIPoint outputPos {0, 0};

    // Initializes and arranges outputs from left to right
    for (LOutput *output : seat()->outputs())
    {
        // Probably a VR headset, meant to be leased by clients
        if (output->isNonDesktop())
        {
            output->setLeasable(true);
            continue;
        }

        // Sets a scale factor of 2 when DPI >= 200
        output->setScale(output->dpi() >= 200 ? 2.f : 1.f);

        // Change it if any of your displays is rotated/flipped
        output->setTransform(CZTransform::Normal);

        // Arrange
        output->setPos(outputPos);

        // Next output x coord
        outputPos.fX = outputPos.x() + output->size().width();

        // Initialize
        addOutput(output);
        output->repaint();
    }
}
//! [initialized]

void LCompositor::loadCursorShapes() noexcept
{
    if (!wellKnownGlobals.CursorShapeManager)
        return;

    const std::vector<std::vector<const char*>> names
    {
     /* Default */        { "left_ptr", "arrow" },
     /* ContextMenu */    { "context-menu", "arrow" },
     /* Help */           { "question_arrow", "help" },
     /* Pointer */        { "hand2", "pointer" },
     /* Progress */       { "watch", "left_ptr_watch" },
     /* Wait */           { "watch", "left_ptr_watch" },
     /* Cell */           { "xterm", "crosshair" },
     /* Crosshair */      { "crosshair", "tcross" },
     /* Text */           { "xterm", "ibeam" },
     /* VerticalText */   { "xterm", "ibeam" },
     /* Alias */          { "alias", "draped_box" },
     /* Copy */           { "copy", "draped_box" },
     /* Move */           { "move", "fleur" },
     /* NoDrop */         { "no-drop", "circle" },
     /* NotAllowed */     { "not-allowed", "circle" },
     /* Grab */           { "grab", "fleur" },
     /* Grabbing */       { "grabbing", "fleur" },
     /* ResizeR */        { "right_side", "e-resize" },
     /* ResizeT */        { "top_side", "n-resize" },
     /* ResizeTR */       { "top_right_corner", "ne-resize" },
     /* ResizeTL */       { "top_left_corner", "nw-resize" },
     /* ResizeB */        { "bottom_side", "s-resize" },
     /* ResizeBR */       { "bottom_right_corner", "se-resize" },
     /* ResizeBL */       { "bottom_left_corner", "sw-resize" },
     /* ResizeL */        { "left_side", "w-resize" },
     /* ResizeLR */       { "sb_h_double_arrow", "h_double_arrow" },
     /* ResizeTB */       { "sb_v_double_arrow", "v_double_arrow" },
     /* ResizeTRBL */     { "top_right_corner", "bottom_left_corner" },
     /* ResizeTLBR */     { "top_left_corner", "bottom_right_corner" },
     /* ResizeColumn */   { "sb_h_double_arrow", "h_double_arrow" },
     /* ResizeRow */      { "sb_v_double_arrow", "v_double_arrow" },
     /* AllScroll */      { "fleur", "cross" },
     /* ZoomIn */         { "zoom-in", "plus" },
     /* ZoomOut */        { "zoom-out", "minus" },
     /* DragAndDropAsk */ { "question_arrow", "hand2" },
     /* MoveOrResize */   { "fleur", "hand2" },
     };

    const char *theme {};
    const Int32 size { 64 };

    for (size_t i = 0; i < names.size(); i++)
    {
        const auto shape { CZCursorShape(i+1) };
        const auto &set { names[i] };

        for (const char *name : set)
        {
            cursor()->setShapeAsset(shape, LCursorSource::MakeFromTheme(name, theme, size));

            if (cursor()->getShapeAsset(shape))
                break;
        }

        if (!cursor()->getShapeAsset(shape))
            log(CZWarning, CZLN, "Could not find a cursor for shape {}", i+1);
    }
}

//! [uninitialized]
void LCompositor::uninitialized() noexcept
{
    /* No default implementation */
}
//! [uninitialized]

//! [createObjectRequest]
LFactoryObject *LCompositor::createObjectRequest(LFactoryObject::Type objectType, const void *params)
{
    CZ_UNUSED(objectType)
    CZ_UNUSED(params)

    /* If nullptr is returned, Louvre creates an instance of the base class */
    return nullptr;
}
//! [createObjectRequest]


//! [onAnticipatedObjectDestruction]
void LCompositor::onAnticipatedObjectDestruction(LFactoryObject *object)
{
    CZ_UNUSED(object)
}
//! [onAnticipatedObjectDestruction]
