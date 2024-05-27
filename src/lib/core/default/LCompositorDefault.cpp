#include <protocols/RelativePointer/GRelativePointerManager.h>
#include <protocols/FractionalScale/GFractionalScaleManager.h>
#include <protocols/PointerConstraints/GPointerConstraints.h>
#include <protocols/TearingControl/GTearingControlManager.h>
#include <protocols/XdgDecoration/GXdgDecorationManager.h>
#include <protocols/GammaControl/GGammaControlManager.h>
#include <protocols/PointerGestures/GPointerGestures.h>
#include <protocols/SessionLock/GSessionLockManager.h>
#include <protocols/PresentationTime/GPresentation.h>
#include <protocols/XdgOutput/GXdgOutputManager.h>
#include <protocols/Wayland/GDataDeviceManager.h>
#include <protocols/LinuxDMABuf/GLinuxDMABuf.h>
#include <protocols/Viewporter/GViewporter.h>
#include <protocols/LayerShell/GLayerShell.h>
#include <protocols/Wayland/GSubcompositor.h>
#include <protocols/XdgShell/GXdgWmBase.h>
#include <protocols/Wayland/GCompositor.h>
#include <protocols/Wayland/GSeat.h>
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
#include <cstring>

#include <protocols/ScreenCopy/GScreenCopyManager.h>

using namespace Louvre;
using namespace Louvre::Protocols;

//! [createGlobalsRequest]
bool LCompositor::createGlobalsRequest()
{
    // Allow clients to create surfaces and regions
    createGlobal<Wayland::GCompositor>();

    // Allow clients to receive pointer, keyboard, and touch events
    createGlobal<Wayland::GSeat>();

    // Provides detailed information of pointer movement
    createGlobal<RelativePointer::GRelativePointerManager>();

    // Allow clients to request setting pointer constraints
    createGlobal<PointerConstraints::GPointerConstraints>();

    // Allow clients to receive swipe, pinch, and hold pointer gestures
    createGlobal<PointerGestures::GPointerGestures>();

    // Enable drag & drop and clipboard data sharing between clients
    createGlobal<Wayland::GDataDeviceManager>();

    // Allow clients to create subsurface roles
    createGlobal<Wayland::GSubcompositor>();

    // Allow clients to create toplevel and popup roles
    createGlobal<XdgShell::GXdgWmBase>();

    // Provides additional info about outputs
    createGlobal<XdgOutput::GXdgOutputManager>();

    // Allow negotiation of server-side or client-side decorations
    createGlobal<XdgDecoration::GXdgDecorationManager>();

    // Allow clients to adjust their surfaces buffers to fractional scales
    createGlobal<FractionalScale::GFractionalScaleManager>();

    // Allow clients to request setting the gamma LUT of outputs
    createGlobal<GammaControl::GGammaControlManager>();

    // Allow clients to create DMA buffers (The Wayland backend currently only supports EGL)
    if (graphicBackendId() == LGraphicBackendDRM)
        createGlobal<LinuxDMABuf::GLinuxDMABuf>();

    // Provides detailed information of how the surfaces are presented
    createGlobal<PresentationTime::GPresentation>();

    // Allows clients to request locking the user session with arbitrary graphics
    createGlobal<SessionLock::GSessionLockManager>();

    // Allows clients to notify their preference of vsync for specific surfaces
    createGlobal<TearingControl::GTearingControlManager>();

    // Allow clients to clip and scale buffers
    createGlobal<Viewporter::GViewporter>();

    createGlobal<ScreenCopy::GScreenCopyManager>();

    createGlobal<LayerShell::GLayerShell>();
    return true;
}
//! [createGlobalsRequest]

bool LCompositor::globalsFilter(LClient *client, LGlobal *global)
{
    L_UNUSED(client)
    L_UNUSED(global)
    return true;
}

//! [initialized]
void LCompositor::initialized()
{
    // Set a keyboard map with "latam" layout
    seat()->keyboard()->setKeymap(nullptr, nullptr, "latam", nullptr);

    Int32 totalWidth = 0;

    // Initialize and arrange outputs from left to right
    for (LOutput *output : seat()->outputs())
    {
        // Set scale 2 to outputs with DPI >= 200
        output->setScale(output->dpi() >= 200 ? 2.f : 1.f);
        output->setTransform(LTransform::Normal);

        output->setPos(LPoint(totalWidth, 0));
        totalWidth += output->size().w();
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

//! [cursorInitialized]
void LCompositor::cursorInitialized()
{
    // Example to load an XCursor

    /*
    // Loads the "hand1" cursor
    LXCursor *handCursor = LXCursor::loadXCursorB("hand1");

    // Returns nullptr if not found
    if (handCursor)
    {
        // Set as the cursor texture
        cursor()->setTextureB(handCursor->texture(), handCursor->hotspotB());
    }
    */
}
//! [cursorInitialized]

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
