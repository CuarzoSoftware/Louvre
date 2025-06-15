#include <CZ/Louvre/Protocols/ImageCaptureSource/GForeignToplevelImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ImageCaptureSource/GOutputImageCaptureSourceManager.h>
#include <CZ/Louvre/Protocols/ForeignToplevelManagement/GForeignToplevelManager.h>
#include <CZ/Louvre/Protocols/SinglePixelBuffer/GSinglePixelBufferManager.h>
#include <CZ/Louvre/Protocols/ForeignToplevelList/GForeignToplevelList.h>
#include <CZ/Louvre/Protocols/RelativePointer/GRelativePointerManager.h>
#include <CZ/Louvre/Protocols/FractionalScale/GFractionalScaleManager.h>
#include <CZ/Louvre/Protocols/InvisibleRegion/GInvisibleRegionManager.h>
#include <CZ/Louvre/Protocols/PointerConstraints/GPointerConstraints.h>
#include <CZ/Louvre/Protocols/TearingControl/GTearingControlManager.h>
#include <CZ/Louvre/Protocols/WlrOutputManagement/GWlrOutputManager.h>
#include <CZ/Louvre/Protocols/BackgroundBlur/GBackgroundBlurManager.h>
#include <CZ/Louvre/Protocols/XdgDecoration/GXdgDecorationManager.h>
#include <CZ/Louvre/Protocols/GammaControl/GGammaControlManager.h>
#include <CZ/Louvre/Protocols/PointerGestures/GPointerGestures.h>
#include <CZ/Louvre/Protocols/SessionLock/GSessionLockManager.h>
#include <CZ/Louvre/Protocols/ContentType/GContentTypeManager.h>
#include <CZ/Louvre/Protocols/IdleInhibit/GIdleInhibitManager.h>
#include <CZ/Louvre/Protocols/PresentationTime/GPresentation.h>
#include <CZ/Louvre/Protocols/ScreenCopy/GScreenCopyManager.h>
#include <CZ/Louvre/Protocols/XdgActivation/GXdgActivation.h>
#include <CZ/Louvre/Protocols/XdgOutput/GXdgOutputManager.h>
#include <CZ/Louvre/Protocols/Wayland/GDataDeviceManager.h>
#include <CZ/Louvre/Protocols/LinuxDMABuf/GLinuxDMABuf.h>
#include <CZ/Louvre/Protocols/IdleNotify/GIdleNotifier.h>
#include <CZ/Louvre/Protocols/SvgPath/GSvgPathManager.h>
#include <CZ/Louvre/Protocols/Viewporter/GViewporter.h>
#include <CZ/Louvre/Protocols/LayerShell/GLayerShell.h>
#include <CZ/Louvre/Protocols/Wayland/GSubcompositor.h>
#include <CZ/Louvre/Protocols/XdgShell/GXdgWmBase.h>
#include <CZ/Louvre/Protocols/Wayland/GCompositor.h>
#include <CZ/Louvre/Protocols/Wayland/GSeat.h>
#include <LSessionLockManager.h>
#include <LSessionLockRole.h>
#include <LCompositor.h>
#include <LToplevelRole.h>
#include <LCursor.h>
#include <LSubsurfaceRole.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LTouch.h>
#include <LSurface.h>
#include <LDND.h>
#include <LClipboard.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPopupRole.h>
#include <LCursorRole.h>
#include <LLog.h>
#include <LXCursor.h>
#include <LClient.h>
#include <LDNDIconRole.h>
#include <LGlobal.h>

using namespace Louvre;
using namespace Louvre::Protocols;

//! [createGlobalsRequest]
bool LCompositor::createGlobalsRequest()
{
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
    createGlobal<GammaControl::GGammaControlManager>();

    // Allow clients to create DMA buffers
    if (!LTexture::supportedDMAFormats().empty())
        createGlobal<LinuxDMABuf::GLinuxDMABuf>();

    // Provides detailed information of how surfaces are presented
    createGlobal<PresentationTime::GPresentation>();

    // Allows clients to request locking the user session with arbitrary graphics
    createGlobal<SessionLock::GSessionLockManager>();

    // Allows clients to notify their preference of vsync for specific surfaces
    createGlobal<TearingControl::GTearingControlManager>();

    // Allows clients to clip and scale buffers
    createGlobal<Viewporter::GViewporter>();

    // Allows clients to capture outputs
    createGlobal<ScreenCopy::GScreenCopyManager>();

    // Allows toplevels to be used as image capture sources
    createGlobal<ImageCaptureSource::GForeignToplevelImageCaptureSourceManager>();

    // Allows outputs to be used as image capture sources
    createGlobal<ImageCaptureSource::GOutputImageCaptureSourceManager>();

    // Allows clients to create wlr_layer_shell surfaces
    createGlobal<LayerShell::GLayerShell>();

    // Allows clients to create single pixel buffers (requires Viewporter::GViewporter)
    createGlobal<SinglePixelBuffer::GSinglePixelBufferManager>();

    // Allows clients to provide a hint about the content type being displayed by surfaces
    createGlobal<ContentType::GContentTypeManager>();

    // Notifies clients if the user has been idle for a given amount of time
    createGlobal<IdleNotify::GIdleNotifier>();

    // Allows clients to request inhibition of the compositor's idle state
    createGlobal<IdleInhibit::GIdleInhibitManager>();

    // Allows clients to activate other client's surfaces
    createGlobal<XdgActivation::GXdgActivation>();

    // Allows clients to arrange/set output properties
    createGlobal<WlrOutputManagement::GWlrOutputManager>();

    /* Experimental */

    // Allows clients to set background blur effects for surfaces
    // createGlobal<BackgroundBlur::GBackgroundBlurManager>();

    // Allows clients to share SVG paths
    // createGlobal<SvgPath::GSvgPathManager>();

    // Allows clients to specify invisible regions within their surfaces
    // createGlobal<InvisibleRegion::GInvisibleRegionManager>();

    return true;
}
//! [createGlobalsRequest]

//! [globalsFilter]
bool LCompositor::globalsFilter(LClient *client, LGlobal *global)
{
    L_UNUSED(client)
    L_UNUSED(global)
    return true;
}
//! [globalsFilter]

//! [onPosixSignal]
void LCompositor::onPosixSignal(int signal)
{
    LLog::debug("[LCompositor::onPosixSignal] Signal %d handled.", signal);
}
//! [onPosixSignal]

//! [initialized]
void LCompositor::initialized()
{
    LPoint outputPos {0, 0};

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
        output->setTransform(LTransform::Normal);

        // Arrange
        output->setPos(outputPos);

        // Next output x coord
        outputPos.setX(outputPos.x() + output->size().w());

        // Initialize
        addOutput(output);
        output->repaint();
    }
}
//! [initialized]

//! [uninitialized]
void LCompositor::uninitialized()
{
    /* No default implementation */
}
//! [uninitialized]

//! [createObjectRequest]
LFactoryObject *LCompositor::createObjectRequest(LFactoryObject::Type objectType, const void *params)
{
    L_UNUSED(objectType)
    L_UNUSED(params)

    /* If nullptr is returned, Louvre creates an instance of the base class */
    return nullptr;
}
//! [createObjectRequest]


//! [onAnticipatedObjectDestruction]
void LCompositor::onAnticipatedObjectDestruction(LFactoryObject *object)
{
    L_UNUSED(object)
}
//! [onAnticipatedObjectDestruction]
