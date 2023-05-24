#include <private/LToplevelRolePrivate.h>
#include <private/LBaseSurfaceRolePrivate.h>

#include <LCompositor.h>
#include <LCursor.h>
#include <LOutput.h>
#include <LSeat.h>
#include <LPointer.h>
#include <LKeyboard.h>

using namespace Louvre;

//! [rolePosC]
const LPoint &LToplevelRole::rolePosC() const
{
    m_rolePosC = surface()->posC() - imp()->currentWindowGeometryC.topLeft();
    return m_rolePosC;
}
//! [rolePosC]

//! [pong]
void LToplevelRole::pong(UInt32)
{
    /* No default implementation */
}
//! [pong]

//! [startMoveRequest]
void LToplevelRole::startMoveRequest()
{
    seat()->pointer()->startMovingToplevelC(this);
}
//! [startMoveRequest]

//! [startResizeRequest]
void LToplevelRole::startResizeRequest(ResizeEdge edge)
{
    seat()->pointer()->startResizingToplevelC(this, edge);
}
//! [startResizeRequest]

//! [configureRequest]
void LToplevelRole::configureRequest()
{
    LOutput *output = compositor()->cursor()->output();

    #if LOUVRE_XDG_WM_BASE_VERSION >= 5

    // Notifies the compositor's capabilities
    setWmCapabilities(WmCapabilities::WmWindowMenu | WmCapabilities::WmMinimize | WmCapabilities::WmMaximize | WmCapabilities::WmFullscreen);

    #endif

    #if LOUVRE_XDG_WM_BASE_VERSION >= 4

    // Suggests to the Toplevel not to use a size larger than the output where the cursor is located
    configureBoundsC(output->sizeC());

    #endif

    // Activates the Toplevel with size (0,0) so that the client can decide the size
    configureC(LSize(0,0), states() | LToplevelRole::Activated);
}
//! [configureRequest]


//! [unsetFullscreenRequest]
void LToplevelRole::unsetFullscreenRequest()
{
    configureC(states() &~ LToplevelRole::Fullscreen);
}
//! [unsetFullscreenRequest]

//! [titleChanged]
void LToplevelRole::titleChanged()
{
    /* No default implementation */
}
//! [titleChanged]


//! [appIdChanged]
void LToplevelRole::appIdChanged()
{
    /* No default implementation */
}
//! [appIdChanged]

//! [geometryChanged]
void LToplevelRole::geometryChanged()
{
    if (this == seat()->pointer()->resizingToplevel())
        seat()->pointer()->updateResizingToplevelPos();
}
//! [geometryChanged]

//! [decorationModeChanged]
void LToplevelRole::decorationModeChanged()
{
    /* No default implementation */
}
//! [decorationModeChanged]


//! [setMaximizedRequest]
void LToplevelRole::setMaximizedRequest()
{
    LOutput *output = compositor()->cursor()->output();
    configureC(output->sizeC(), LToplevelRole::Activated);
    configureC(output->sizeC(), LToplevelRole::Activated | LToplevelRole::Maximized);
}
//! [setMaximizedRequest]

//! [unsetMaximizedRequest]
void LToplevelRole::unsetMaximizedRequest()
{
    configureC(states() &~ LToplevelRole::Maximized);
}
//! [unsetMaximizedRequest]

//! [maximizedChanged]
void LToplevelRole::maximizedChanged()
{
    LOutput *output = compositor()->cursor()->output();

    if (maximized())
    {
        compositor()->raiseSurface(surface());
        surface()->setPosC(output->posC());
        surface()->setMinimized(false);
    }
}
//! [maximizedChanged]

//! [fullscreenChanged]
void LToplevelRole::fullscreenChanged()
{
    if (fullscreen())
    {
        surface()->setPosC(compositor()->cursor()->output()->posC());
        compositor()->raiseSurface(surface());
    }
}
//! [fullscreenChanged]

//! [activatedChanged]
void LToplevelRole::activatedChanged()
{
    if (activated())
        seat()->keyboard()->setFocus(surface());
}
//! [activatedChanged]

//! [maxSizeChanged]
void LToplevelRole::maxSizeChanged()
{
    /* No default implementation */
}
//! [maxSizeChanged]

//! [minSizeChanged]
void LToplevelRole::minSizeChanged()
{
    /* No default implementation */
}
//! [minSizeChanged]

//! [setMinimizedRequest]
void LToplevelRole::setMinimizedRequest()
{
    surface()->setMinimized(true);

    if (surface() == seat()->pointer()->focusSurface())
        seat()->pointer()->setFocusC(nullptr);

    if (surface() == seat()->keyboard()->focusSurface())
        seat()->keyboard()->setFocus(nullptr);

    if (this == seat()->pointer()->movingToplevel())
        seat()->pointer()->stopMovingToplevel();

    if (this == seat()->pointer()->resizingToplevel())
        seat()->pointer()->stopResizingToplevel();
}
//! [setMinimizedRequest]

//! [setFullscreenRequest]
void LToplevelRole::setFullscreenRequest(LOutput *destOutput)
{
    LOutput *output;

    if (destOutput)
        output = destOutput;
    else
        output = compositor()->cursor()->output();


    configureC(output->sizeC(), LToplevelRole::Activated | LToplevelRole::Fullscreen);
}
//! [setFullscreenRequest]

//! [showWindowMenuRequestS]
void LToplevelRole::showWindowMenuRequestS(Int32 x, Int32 y)
{
    L_UNUSED(x);
    L_UNUSED(y);
}
//! [showWindowMenuRequestS]
