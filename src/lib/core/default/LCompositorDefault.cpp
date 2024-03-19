#include <protocols/XdgShell/private/GXdgWmBasePrivate.h>
#include <protocols/XdgDecoration/private/GXdgDecorationManagerPrivate.h>
#include <protocols/LinuxDMABuf/private/GLinuxDMABufPrivate.h>
#include <protocols/PresentationTime/private/GPresentationPrivate.h>
#include <protocols/Viewporter/private/GViewporterPrivate.h>
#include <protocols/FractionalScale/private/GFractionalScaleManagerPrivate.h>
#include <protocols/GammaControl/private/GGammaControlManagerPrivate.h>
#include <protocols/TearingControl/private/GTearingControlManagerPrivate.h>
#include <protocols/RelativePointer/private/GRelativePointerManagerPrivate.h>
#include <protocols/PointerGestures/private/GPointerGesturesPrivate.h>

#include <protocols/Wayland/GSubcompositor.h>
#include <protocols/Wayland/GCompositor.h>
#include <protocols/Wayland/GDataDeviceManager.h>
#include <protocols/Wayland/GSeat.h>

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
#include <cstring>

using namespace Louvre;
using namespace Louvre::Protocols;

//! [createGlobalsRequest]
bool LCompositor::createGlobalsRequest()
{
    wl_global_create(display(), &wl_compositor_interface,
                     LOUVRE_WL_COMPOSITOR_VERSION, this, &Wayland::GCompositor::bind);

    wl_global_create(display(), &wl_seat_interface,
                     LOUVRE_WL_SEAT_VERSION, this, &Wayland::GSeat::bind);

    wl_global_create(display(), &wl_subcompositor_interface,
                     LOUVRE_WL_SUBCOMPOSITOR_VERSION, this, &Wayland::GSubcompositor::bind);

    wl_global_create(display(), &wl_data_device_manager_interface,
                     LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION, this, &Wayland::GDataDeviceManager::bind);

    wl_global_create(display(), &xdg_wm_base_interface,
                     LOUVRE_XDG_WM_BASE_VERSION, this, &GXdgWmBase::GXdgWmBasePrivate::bind);

    wl_global_create(display(), &zxdg_decoration_manager_v1_interface,
                     LOUVRE_XDG_DECORATION_MANAGER_VERSION, this, &GXdgDecorationManager::GXdgDecorationManagerPrivate::bind);

    wl_global_create(display(), &zwp_linux_dmabuf_v1_interface,
                     LOUVRE_LINUX_DMA_BUF_VERSION, this, &GLinuxDMABuf::GLinuxDMABufPrivate::bind);

    wl_global_create(display(), &wp_presentation_interface,
                     LOUVRE_PRESENTATION_VERSION, this, &GPresentation::GPresentationPrivate::bind);

    wl_global_create(display(), &wp_viewporter_interface,
                     LOUVRE_VIEWPORTER_VERSION, this, &GViewporter::GViewporterPrivate::bind);

    wl_global_create(display(), &wp_fractional_scale_manager_v1_interface,
                     LOUVRE_FRACTIONAL_SCALE_VERSION, this, &GFractionalScaleManager::GFractionalScaleManagerPrivate::bind);

    wl_global_create(display(), &zwlr_gamma_control_manager_v1_interface,
                     LOUVRE_GAMMA_CONTROL_MANAGER_VERSION, this, &GGammaControlManager::GGammaControlManagerPrivate::bind);

    wl_global_create(display(), &wp_tearing_control_manager_v1_interface,
                     LOUVRE_TEARING_CONTROL_MANAGER_VERSION, this, &GTearingControlManager::GTearingControlManagerPrivate::bind);

    wl_global_create(display(), &zwp_relative_pointer_manager_v1_interface,
                     LOUVRE_RELATIVE_POINTER_MANAGER_VERSION, this, &GRelativePointerManager::GRelativePointerManagerPrivate::bind);

    wl_global_create(display(), &zwp_pointer_gestures_v1_interface,
                     LOUVRE_POINTER_GESTURES_VERSION, this, &GPointerGestures::GPointerGesturesPrivate::bind);

    wl_display_init_shm(display());

    return true;
}
//! [createGlobalsRequest]

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
        output->setTransform(LFramebuffer::Normal);

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

//! [createOutputRequest]
LOutput *LCompositor::createOutputRequest(const void *params)
{
    return new LOutput(params);
}
//! [createOutputRequest]

//! [createClientRequest]
LClient *LCompositor::createClientRequest(const void *params)
{
    return new LClient(params);
}
//! [createClientRequest]

//! [createSurfaceRequest]
LSurface *LCompositor::createSurfaceRequest(const void *params)
{
    return new LSurface(params);
}
//! [createSurfaceRequest]

//! [createSeatRequest]
LSeat *LCompositor::createSeatRequest(const void *params)
{
    return new LSeat(params);
}
//! [createSeatRequest]

//! [createPointerRequest]
LPointer *LCompositor::createPointerRequest(const void *params)
{
    return new LPointer(params);
}
//! [createPointerRequest]

//! [createKeyboardRequest]
LKeyboard *LCompositor::createKeyboardRequest(const void *params)
{
    return new LKeyboard(params);
}
//! [createKeyboardRequest]

//! [createTouchRequest]
LTouch *LCompositor::createTouchRequest(const void *params)
{
    return new LTouch(params);
}
//! [createTouchRequest]

//! [createDNDRequest]
LDND *LCompositor::createDNDRequest(const void *params)
{
    return new LDND(params);
}
//! [createDNDRequest]

//! [createClipboardRequest]
LClipboard *LCompositor::createClipboardRequest(const void *params)
{
    return new LClipboard(params);
}
//! [createClipboardRequest]

//! [createToplevelRoleRequest]
LToplevelRole *LCompositor::createToplevelRoleRequest(const void *params)
{
    return new LToplevelRole(params);
}
//! [createToplevelRoleRequest]

//! [createPopupRoleRequest]
LPopupRole *LCompositor::createPopupRoleRequest(const void *params)
{
    return new LPopupRole(params);
}
//! [createPopupRoleRequest]

//! [createSubsurfaceRoleRequest]
LSubsurfaceRole *LCompositor::createSubsurfaceRoleRequest(const void *params)
{
    return new LSubsurfaceRole(params);
}
//! [createSubsurfaceRoleRequest]

//! [createCursorRoleRequest]
LCursorRole *LCompositor::createCursorRoleRequest(const void *params)
{
    return new LCursorRole(params);
}
//! [createCursorRoleRequest]

//! [createDNDIconRoleRequest]
LDNDIconRole *LCompositor::createDNDIconRoleRequest(const void *params)
{
    return new LDNDIconRole(params);
}
//! [createDNDIconRoleRequest]

//! [destroyOutputRequest]
void LCompositor::destroyOutputRequest(LOutput *output)
{
    L_UNUSED(output);
}
//! [destroyOutputRequest]

//! [destroyClientRequest]
void LCompositor::destroyClientRequest(LClient *client)
{
    L_UNUSED(client);
}
//! [destroyClientRequest]

//! [destroySurfaceRequest]
void LCompositor::destroySurfaceRequest(LSurface *surface)
{
    L_UNUSED(surface);
}
//! [destroySurfaceRequest]

//! [destroySeatRequest]
void LCompositor::destroySeatRequest(LSeat *seat)
{
    L_UNUSED(seat);
}
//! [destroySeatRequest]

//! [destroyPointerRequest]
void LCompositor::destroyPointerRequest(LPointer *pointer)
{
    L_UNUSED(pointer);
}
//! [destroyPointerRequest]

//! [destroyTouchRequest]
void LCompositor::destroyTouchRequest(LTouch *touch)
{
    L_UNUSED(touch);
}
//! [destroyTouchRequest]

//! [destroyKeyboardRequest]
void LCompositor::destroyKeyboardRequest(LKeyboard *keyboard)
{
    L_UNUSED(keyboard);
}
//! [destroyKeyboardRequest]

//! [destroyDNDRequest]
void LCompositor::destroyDNDRequest(LDND *dnd)
{
    L_UNUSED(dnd);
}
//! [destroyDNDRequest]

//! [destroyClipboardRequest]
void LCompositor::destroyClipboardRequest(LClipboard *clipboard)
{
    L_UNUSED(clipboard);
}
//! [destroyClipboardRequest]

//! [destroyToplevelRoleRequest]
void LCompositor::destroyToplevelRoleRequest(LToplevelRole *toplevel)
{
    L_UNUSED(toplevel);
}
//! [destroyToplevelRoleRequest]

//! [destroyPopupRoleRequest]
void LCompositor::destroyPopupRoleRequest(LPopupRole *popup)
{
    L_UNUSED(popup);
}
//! [destroyPopupRoleRequest]

//! [destroySubsurfaceRoleRequest]
void LCompositor::destroySubsurfaceRoleRequest(LSubsurfaceRole *subsurface)
{
    L_UNUSED(subsurface);
}
//! [destroySubsurfaceRoleRequest]

//! [destroyCursorRoleRequest]
void LCompositor::destroyCursorRoleRequest(LCursorRole *cursorRole)
{
     L_UNUSED(cursorRole);
}
//! [destroyCursorRoleRequest]

//! [destroyDNDIconRoleRequest]
void LCompositor::destroyDNDIconRoleRequest(LDNDIconRole *icon)
{
    L_UNUSED(icon)
}
//! [destroyDNDIconRoleRequest]
