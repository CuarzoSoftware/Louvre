#include <LCompositor.h>
#include <LToplevelRole.h>
#include <LCursor.h>
#include <LSubsurfaceRole.h>
#include <LPointer.h>
#include <LKeyboard.h>
#include <LSurface.h>
#include <LDNDManager.h>
#include <LOutputManager.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPopupRole.h>
#include <LCursorRole.h>
#include <LLog.h>

#include <LXCursor.h>

using namespace Louvre;

//! [initialized]
void LCompositor::initialized()
{

    // Change the keyboard map to "latam"
    seat()->keyboard()->setKeymap( NULL, NULL, "latam", NULL);

    // Use the Output Manager to get avaliable outputs
    if(outputManager()->outputs()->empty())
    {
        LLog::fatal("No output available.");
        finish();
    }

    // Set double scale to outputs with DPI >= 120
    for(LOutput *output : *outputManager()->outputs())
    {
        if(output->dpi() >= 120)
            output->setScale(2);

        addOutput(output);
    }


    // Organize outputs horizontally and sequentially.

    Int32 totalWidth = 0;

    for(LOutput *output : outputs())
    {
        output->setPosC(LPoint(totalWidth,0));
        totalWidth += output->sizeC().w();
    }

}
//! [initialized]

//! [cursorInitialized]
void LCompositor::cursorInitialized()
{
    // Example to load an XCursor

    /*

    // Loads the "hand1" cursor
    LXCursor *handCursor = cursor()->loadXCursorB("hand1");

    // Returns nullptr if not found
    if(handCursor)
    {
        // Set as the cursor texture
        cursor()->setTextureB(handCursor->texture(), handCursor->hotspotB());
    }

    */
}
//! [cursorInitialized]

//! [globalScaleChanged]
void LCompositor::globalScaleChanged(Int32 oldScale, Int32 newScale)
{
    L_UNUSED(oldScale);
    L_UNUSED(newScale);
}
//! [globalScaleChanged]

//! [createOutputManagerRequest]
LOutputManager *LCompositor::createOutputManagerRequest(LOutputManager::Params *params)
{
    return new LOutputManager(params);
}
//! [createOutputManagerRequest]


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

//! [destroyToplevelRoleRequest]
void LCompositor::destroyToplevelRoleRequest(LToplevelRole *toplevel)
{
    L_UNUSED(toplevel);
}
//! [destroyToplevelRoleRequest]

//! [destroyPopupRoleRequest]
void LCompositor::destroyPopupRoleRequest(LPopupRole *popup)
{
    // Return implicit grab to parent
    if(popup->surface()->parent() && (popup->surface() == seat()->keyboard()->focusSurface() || popup->surface() == seat()->pointer()->focusSurface()))
        seat()->keyboard()->setFocus(popup->surface()->parent());
    
    repaintAllOutputs();
}
//! [destroyPopupRoleRequest]

//! [destroyCursorRoleRequest]
void LCompositor::destroyCursorRoleRequest(LCursorRole *cursorRole)
{
    if(cursorRole->surface()->texture() == cursor()->texture())
        cursor()->useDefault();
}
//! [destroyCursorRoleRequest]

//! [destroyDNDIconRoleRequest]
void LCompositor::destroyDNDIconRoleRequest(LDNDIconRole *icon)
{
    L_UNUSED(icon)
}
//! [destroyDNDIconRoleRequest]
