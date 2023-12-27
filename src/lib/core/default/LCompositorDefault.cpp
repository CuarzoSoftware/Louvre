#include <cstring>
#include <protocols/Wayland/private/GSeatPrivate.h>
#include <protocols/Wayland/private/GCompositorPrivate.h>
#include <protocols/Wayland/private/GSubcompositorPrivate.h>
#include <protocols/Wayland/private/GDataDeviceManagerPrivate.h>
#include <protocols/XdgShell/private/GXdgWmBasePrivate.h>
#include <protocols/XdgDecoration/private/GXdgDecorationManagerPrivate.h>
#include <protocols/LinuxDMABuf/private/GLinuxDMABufPrivate.h>
#include <protocols/WpPresentationTime/private/GWpPresentationPrivate.h>
#include <LCompositor.h>
#include <LToplevelRole.h>
#include <LCursor.h>
#include <LSubsurfaceRole.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LSurface.h>
#include <LDNDManager.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPopupRole.h>
#include <LCursorRole.h>
#include <LLog.h>
#include <LXCursor.h>

using namespace Louvre;

//! [createGlobalsRequest]
bool LCompositor::createGlobalsRequest()
{
    wl_global_create(display(), &wl_compositor_interface,
                     LOUVRE_WL_COMPOSITOR_VERSION, this, &GCompositor::GCompositorPrivate::bind);

    wl_global_create(display(), &wl_seat_interface,
                     LOUVRE_WL_SEAT_VERSION, this, &GSeat::GSeatPrivate::bind);

    wl_global_create(display(), &wl_subcompositor_interface,
                     LOUVRE_WL_SUBCOMPOSITOR_VERSION, this, &GSubcompositor::GSubcompositorPrivate::bind);

    wl_global_create(display(), &wl_data_device_manager_interface,
                     LOUVRE_WL_DATA_DEVICE_MANAGER_VERSION, this, &GDataDeviceManager::GDataDeviceManagerPrivate::bind);

    wl_global_create(display(), &xdg_wm_base_interface,
                     LOUVRE_XDG_WM_BASE_VERSION, this, &Protocols::XdgShell::GXdgWmBase::GXdgWmBasePrivate::bind);

    wl_global_create(display(), &zxdg_decoration_manager_v1_interface,
                     LOUVRE_XDG_DECORATION_MANAGER_VERSION, this, &Protocols::XdgDecoration::GXdgDecorationManager::GXdgDecorationManagerPrivate::bind);

    wl_global_create(display(), &zwp_linux_dmabuf_v1_interface,
                     LOUVRE_LINUX_DMA_BUF_VERSION, this, &Protocols::LinuxDMABuf::GLinuxDMABuf::GLinuxDMABufPrivate::bind);

    wl_global_create(display(), &wp_presentation_interface,
                     LOUVRE_WP_PRESENTATION_VERSION, this, &Protocols::WpPresentationTime::GWpPresentation::GWpPresentationPrivate::bind);

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
LOutput *LCompositor::createOutputRequest()
{
    return new LOutput();
}
//! [createOutputRequest]

//! [createClientRequest]
LClient *LCompositor::createClientRequest(LClient::Params *params)
{
    return new LClient(params);
}
//! [createClientRequest]

//! [createSurfaceRequest]
LSurface *LCompositor::createSurfaceRequest(LSurface::Params *params)
{
    return new LSurface(params);
}
//! [createSurfaceRequest]

//! [createSeatRequest]
LSeat *LCompositor::createSeatRequest(LSeat::Params *params)
{
    return new LSeat(params);
}
//! [createSeatRequest]

//! [createPointerRequest]
LPointer *LCompositor::createPointerRequest(LPointer::Params *params)
{
    return new LPointer(params);
}
//! [createPointerRequest]

//! [createKeyboardRequest]
LKeyboard *LCompositor::createKeyboardRequest(LKeyboard::Params *params)
{
    return new LKeyboard(params);
}
//! [createKeyboardRequest]

//! [createDNDManagerRequest]
LDNDManager *LCompositor::createDNDManagerRequest(LDNDManager::Params *params)
{
    return new LDNDManager(params);
}
//! [createDNDManagerRequest]

//! [createToplevelRoleRequest]
LToplevelRole *LCompositor::createToplevelRoleRequest(LToplevelRole::Params *params)
{
    return new LToplevelRole(params);
}
//! [createToplevelRoleRequest]

//! [createPopupRoleRequest]
LPopupRole *LCompositor::createPopupRoleRequest(LPopupRole::Params *params)
{
    return new LPopupRole(params);
}
//! [createPopupRoleRequest]

//! [createSubsurfaceRoleRequest]
LSubsurfaceRole *LCompositor::createSubsurfaceRoleRequest(LSubsurfaceRole::Params *params)
{
    return new LSubsurfaceRole(params);
}
//! [createSubsurfaceRoleRequest]

//! [createCursorRoleRequest]
LCursorRole *LCompositor::createCursorRoleRequest(LCursorRole::Params *params)
{
    return new LCursorRole(params);
}
//! [createCursorRoleRequest]

//! [createDNDIconRoleRequest]
LDNDIconRole *LCompositor::createDNDIconRoleRequest(LDNDIconRole::Params *params)
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

//! [destroyKeyboardRequest]
void LCompositor::destroyKeyboardRequest(LKeyboard *keyboard)
{
    L_UNUSED(keyboard);
}
//! [destroyKeyboardRequest]

//! [destroyDNDManagerRequest]
void LCompositor::destroyDNDManagerRequest(LDNDManager *dndManager)
{
    L_UNUSED(dndManager);
}
//! [destroyDNDManagerRequest]

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
