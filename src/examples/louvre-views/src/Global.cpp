#include <LLog.h>
#include <string.h>
#include <LXCursor.h>
#include <LOpenGL.h>

#include "Global.h"
#include "Compositor.h"
#include "Output.h"
#include "Dock.h"
#include "TextRenderer.h"

static G::DockTextures _dockTextures;
static G::ToplevelTextures _toplevelTextures;
static G::Cursors xCursors;
static G::Fonts _fonts;

Compositor *G::compositor()
{
    return (Compositor*)LCompositor::compositor();
}

LScene *G::scene()
{
    return compositor()->scene;
}

Pointer *G::pointer()
{
    return (Pointer*)compositor()->seat()->pointer();
}

std::list<Output *> &G::outputs()
{
    return (std::list<Output*>&)compositor()->outputs();
}

void G::loadDockTextures()
{
    _dockTextures.left = LOpenGL::loadTexture("/usr/etc/Louvre/assets/dock_side.png");

    if (!_dockTextures.left)
    {
        LLog::fatal("[louvre-views] Failed to load dock_side.png texture.");
        exit(1);
    }

    _dockTextures.center = LOpenGL::loadTexture("/usr/etc/Louvre/assets/dock_clamp.png");

    if (!_dockTextures.center)
    {
        LLog::fatal("[louvre-views] Failed to load dock_center.png texture.");
        exit(1);
    }

    _dockTextures.right = _dockTextures.left->copyB(_dockTextures.left->sizeB(),
                                                    LRect(0,
                                                          0,
                                                          - _dockTextures.left->sizeB().w(),
                                                          _dockTextures.left->sizeB().h()));
}

G::DockTextures &G::dockTextures()
{
    return _dockTextures;
}

void G::enableDocks(bool enabled)
{
    for (Output *o : outputs())
        if (o->dock)
            o->dock->setVisible(enabled);
}

void G::loadCursors()
{
    xCursors.hand2 = LXCursor::loadXCursorB("hand2");
    xCursors.top_left_corner = LXCursor::loadXCursorB("top_left_corner");
    xCursors.top_right_corner= LXCursor::loadXCursorB("top_right_corner");
    xCursors.bottom_left_corner = LXCursor::loadXCursorB("bottom_left_corner");
    xCursors.bottom_right_corner = LXCursor::loadXCursorB("bottom_right_corner");
    xCursors.left_side = LXCursor::loadXCursorB("left_side");
    xCursors.top_side = LXCursor::loadXCursorB("top_side");
    xCursors.right_side = LXCursor::loadXCursorB("right_side");
    xCursors.bottom_side = LXCursor::loadXCursorB("bottom_side");
}

G::Cursors &G::cursors()
{
    return xCursors;
}

void G::loadToplevelTextures()
{
    _toplevelTextures.activeTL = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_active_top_left.png");
    _toplevelTextures.activeT = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_active_top_clamp.png");
    _toplevelTextures.activeTR = _toplevelTextures.activeTL->copyB(_toplevelTextures.activeTL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.activeTL->sizeB().w(),
                                                                        _toplevelTextures.activeTL->sizeB().h()));
    _toplevelTextures.activeL = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_active_side_clamp.png");
    _toplevelTextures.activeR = _toplevelTextures.activeL->copyB(_toplevelTextures.activeL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.activeL->sizeB().w(),
                                                                        _toplevelTextures.activeL->sizeB().h()));
    _toplevelTextures.activeBL = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_active_bottom_left.png");
    _toplevelTextures.activeB = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_active_bottom_clamp.png");
    _toplevelTextures.activeBR = _toplevelTextures.activeBL->copyB(_toplevelTextures.activeBL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.activeBL->sizeB().w(),
                                                                        _toplevelTextures.activeBL->sizeB().h()));

    _toplevelTextures.inactiveTL = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_inactive_top_left.png");
    _toplevelTextures.inactiveT = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_inactive_top_clamp.png");
    _toplevelTextures.inactiveTR = _toplevelTextures.inactiveTL->copyB(_toplevelTextures.inactiveTL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.inactiveTL->sizeB().w(),
                                                                        _toplevelTextures.inactiveTL->sizeB().h()));
    _toplevelTextures.inactiveL = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_inactive_side_clamp.png");
    _toplevelTextures.inactiveR = _toplevelTextures.inactiveL->copyB(_toplevelTextures.inactiveL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.inactiveL->sizeB().w(),
                                                                        _toplevelTextures.inactiveL->sizeB().h()));
    _toplevelTextures.inactiveBL = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_inactive_bottom_left.png");
    _toplevelTextures.inactiveB = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_inactive_bottom_clamp.png");
    _toplevelTextures.inactiveBR = _toplevelTextures.inactiveBL->copyB(_toplevelTextures.inactiveBL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.inactiveBL->sizeB().w(),
                                                                        _toplevelTextures.inactiveBL->sizeB().h()));


    _toplevelTextures.maskBL = LOpenGL::loadTexture("/usr/etc/Louvre/assets/toplevel_border_radius_mask.png");
    _toplevelTextures.maskBR = _toplevelTextures.maskBL->copyB(_toplevelTextures.maskBL->sizeB(),
                                                                  LRect(0,
                                                                        0,
                                                                        -_toplevelTextures.maskBL->sizeB().w(),
                                                                        _toplevelTextures.maskBL->sizeB().h()));

    LRect activeTransRectsTL[] = TOPLEVEL_ACTIVE_TOP_LEFT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(activeTransRectsTL)/sizeof(LRect); i++)
        _toplevelTextures.activeTransRegionTL.addRect(activeTransRectsTL[i]);

    LRect activeTransRectsTR[] = TOPLEVEL_ACTIVE_TOP_RIGHT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(activeTransRectsTR)/sizeof(LRect); i++)
        _toplevelTextures.activeTransRegionTR.addRect(activeTransRectsTR[i]);


    LRect inactiveTransRectsTL[] = TOPLEVEL_INACTIVE_TOP_LEFT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(inactiveTransRectsTL)/sizeof(LRect); i++)
        _toplevelTextures.inactiveTransRegionTL.addRect(inactiveTransRectsTL[i]);

    LRect inactiveTransRectsTR[] = TOPLEVEL_INACTIVE_TOP_RIGHT_TRANS_REGION;

    for (UInt64 i = 0; i < sizeof(inactiveTransRectsTR)/sizeof(LRect); i++)
        _toplevelTextures.inactiveTransRegionTR.addRect(inactiveTransRectsTR[i]);

    _toplevelTextures.inactiveButton = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_inactive.png");
    _toplevelTextures.activeCloseButton = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_close.png");
    _toplevelTextures.activeCloseButtonHover = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_close_hover.png");
    _toplevelTextures.activeCloseButtonPressed = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_close_pressed.png");
    _toplevelTextures.activeMinimizeButton = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_minimize.png");
    _toplevelTextures.activeMinimizeButtonHover = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_minimize_hover.png");
    _toplevelTextures.activeMinimizeButtonPressed = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_minimize_pressed.png");
    _toplevelTextures.activeMaximizeButton = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_maximize.png");
    _toplevelTextures.activeMaximizeButtonHover = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_maximize_hover.png");
    _toplevelTextures.activeMaximizeButtonPressed = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_maximize_pressed.png");
    _toplevelTextures.activeFullscreenButtonHover = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_fullscreen_hover.png");
    _toplevelTextures.activeFullscreenButtonPressed = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_fullscreen_pressed.png");
    _toplevelTextures.activeUnfullscreenButtonHover = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_unfullscreen_hover.png");
    _toplevelTextures.activeUnfullscreenButtonPressed = LOpenGL::loadTexture("/usr/etc/Louvre/assets/button_unfullscreen_pressed.png");
}

G::ToplevelTextures &G::toplevelTextures()
{
    return _toplevelTextures;
}

void G::loadFonts()
{
    _fonts.regular = TextRenderer::loadFont("Inter");
    _fonts.semibold = TextRenderer::loadFont("Inter Semi Bold");
}

G::Fonts *G::font()
{
    return &_fonts;
}
