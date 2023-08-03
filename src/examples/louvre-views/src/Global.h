#ifndef SHARED_H
#define SHARED_H

class Compositor;
class Output;

#include <LNamespaces.h>
#include <LRegion.h>

using namespace Louvre;

#define DOCK_SHADOW_SIZE 40
#define DOCK_HEIGHT 53
#define DOCK_SPACING 8
#define DOCK_PADDING 8
#define DOCK_MARGIN 8
#define DOCK_BORDER_RADIUS 16
#define DOCK_ITEM_HEIGHT (DOCK_HEIGHT - (2 * DOCK_PADDING))

#define TOPBAR_HEIGHT 24

#define TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_X -48
#define TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_Y -61
#define TOPLEVEL_ACTIVE_TOP_CLAMP_OFFSET_Y 1
#define TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_X -48
#define TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_Y -21
#define TOPLEVEL_ACTIVE_MIN_WIDTH 68
#define TOPLEVEL_ACTIVE_MIN_WIDTH_TOP 44
#define TOPLEVEL_ACTIVE_MIN_WIDTH_BOTTOM 68
#define TOPLEVEL_ACTIVE_MIN_HEIGHT 34
#define TOPLEVEL_ACTIVE_TOP_LEFT_TRANS_REGION {LRect(0,0,48,74),LRect(48,0,22,32),LRect(48,32,11,11),LRect(48,61,22,13)}
#define TOPLEVEL_ACTIVE_TOP_RIGHT_TRANS_REGION {LRect(22,0,48,74),LRect(0,0,22,32),LRect(11,32,11,11),LRect(0,61,22,13)}
#define TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_X -41
#define TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_Y -56
#define TOPLEVEL_INACTIVE_TOP_CLAMP_OFFSET_Y 1
#define TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_X -41
#define TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_Y -14
#define TOPLEVEL_INACTIVE_MIN_WIDTH 60
#define TOPLEVEL_INACTIVE_MIN_WIDTH_TOP 30
#define TOPLEVEL_INACTIVE_MIN_WIDTH_BOTTOM 60
#define TOPLEVEL_INACTIVE_MIN_HEIGHT 21
#define TOPLEVEL_INACTIVE_TOP_LEFT_TRANS_REGION {LRect(0,0,41,63),LRect(41,0,15,27),LRect(41,27,11,11),LRect(41,56,15,7)}
#define TOPLEVEL_INACTIVE_TOP_RIGHT_TRANS_REGION {LRect(15,0,41,63),LRect(0,0,15,27),LRect(4,27,11,11),LRect(0,56,15,7)}
#define TOPLEVEL_TOPBAR_HEIGHT 29
#define TOPLEVEL_BORDER_RADIUS 11
#define TOPLEVEL_BUTTON_SIZE 12
#define TOPLEVEL_BUTTON_SPACING 8
#define TOPLEVEL_RESIZE_INPUT_MARGIN 4

class G
{
public:

    struct Cursors
    {
        LXCursor *hand2 = nullptr;
        LXCursor *top_left_corner = nullptr;
        LXCursor *top_right_corner = nullptr;
        LXCursor *bottom_left_corner = nullptr;
        LXCursor *bottom_right_corner = nullptr;
        LXCursor *left_side = nullptr;
        LXCursor *top_side = nullptr;
        LXCursor *right_side = nullptr;
        LXCursor *bottom_side = nullptr;
    };

    struct DockTextures
    {
        LTexture *left = nullptr;
        LTexture *center = nullptr;
        LTexture *right = nullptr;
    };

    struct ToplevelTextures
    {
        LTexture *activeTL = nullptr;
        LTexture *activeT = nullptr;
        LTexture *activeTR = nullptr;
        LTexture *activeL = nullptr;
        LTexture *activeR = nullptr;
        LTexture *activeBL = nullptr;
        LTexture *activeB = nullptr;
        LTexture *activeBR = nullptr;

        LTexture *inactiveTL = nullptr;
        LTexture *inactiveT = nullptr;
        LTexture *inactiveTR = nullptr;
        LTexture *inactiveL = nullptr;
        LTexture *inactiveR = nullptr;
        LTexture *inactiveBL = nullptr;
        LTexture *inactiveB = nullptr;
        LTexture *inactiveBR = nullptr;

        LTexture *maskBL = nullptr;
        LTexture *maskBR = nullptr;

        LTexture *inactiveButton = nullptr;
        LTexture *activeCloseButton = nullptr;
        LTexture *activeCloseButtonHover = nullptr;
        LTexture *activeCloseButtonPressed = nullptr;
        LTexture *activeMinimizeButton = nullptr;
        LTexture *activeMinimizeButtonHover = nullptr;
        LTexture *activeMinimizeButtonPressed = nullptr;
        LTexture *activeMaximizeButton = nullptr;
        LTexture *activeMaximizeButtonHover = nullptr;
        LTexture *activeMaximizeButtonPressed = nullptr;
        LTexture *activeFullscreenButtonHover = nullptr;
        LTexture *activeFullscreenButtonPressed = nullptr;
        LTexture *activeUnfullscreenButtonHover = nullptr;
        LTexture *activeUnfullscreenButtonPressed = nullptr;

        LRegion activeTransRegionTL;
        LRegion activeTransRegionTR;
        LRegion inactiveTransRegionTL;
        LRegion inactiveTransRegionTR;
    };

    // Quick handles
    static Compositor *compositor();
    static LScene *scene();
    static std::list<Output*>&outputs();

    // Dock
    static void loadDockTextures();
    static DockTextures &dockTextures();

    // Cursors
    static void loadCursors();
    static Cursors &cursors();

    // Toplevel
    static void loadToplevelTextures();
    static ToplevelTextures &toplevelTextures();
};

#endif // SHARED_H
