#ifndef SHARED_H
#define SHARED_H

class Compositor;
class Output;

#include <LNamespaces.h>
#include <LRegion.h>

using namespace Louvre;

#define DOCK_BUFFER_SCALE 2
#define DOCK_SHADOW_SIZE 40
#define DOCK_HEIGHT 53
#define DOCK_SPACING 8
#define DOCK_PADDING 8
#define DOCK_MARGIN 8
#define DOCK_BORDER_RADIUS 16
#define DOCK_ITEM_HEIGHT (DOCK_HEIGHT - (2 * DOCK_PADDING))


#define TOPLEVEL_RESIZE_INPUT_MARGIN 4

#define TOPBAR_HEIGHT 24

#define TOPLEVEL_TOPBAR_BUTTON_SIZE 12

#define TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_X -41
#define TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_Y -56
#define TOPLEVEL_ACTIVE_TOP_CLAMP_OFFSET_Y 1
#define TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_X -41
#define TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_Y -15
#define TOPLEVEL_ACTIVE_MIN_WIDTH 60
#define TOPLEVEL_ACTIVE_MIN_WIDTH_TOP 30
#define TOPLEVEL_ACTIVE_MIN_WIDTH_BOTTOM 60
#define TOPLEVEL_ACTIVE_MIN_HEIGHT 23
#define TOPLEVEL_ACTIVE_TOP_LEFT_TRANS_REGION {LRect(0,0,41,64),LRect(41,0,15,27),LRect(41,27,11,11),LRect(41,56,15,8)}
#define TOPLEVEL_ACTIVE_TOP_RIGHT_TRANS_REGION {LRect(15,0,41,64),LRect(0,0,15,27),LRect(4,27,11,11),LRect(0,56,15,8)}
#define TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_X -37
#define TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_Y -53
#define TOPLEVEL_INACTIVE_TOP_CLAMP_OFFSET_Y 1
#define TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_X -37
#define TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_Y -12
#define TOPLEVEL_INACTIVE_MIN_WIDTH 46
#define TOPLEVEL_INACTIVE_MIN_WIDTH_TOP 22
#define TOPLEVEL_INACTIVE_MIN_WIDTH_BOTTOM 46
#define TOPLEVEL_INACTIVE_MIN_HEIGHT 14
#define TOPLEVEL_INACTIVE_TOP_LEFT_TRANS_REGION {LRect(0,0,37,55),LRect(37,0,11,24),LRect(37,24,11,11),LRect(37,53,11,2)}
#define TOPLEVEL_INACTIVE_TOP_RIGHT_TRANS_REGION {LRect(11,0,37,55),LRect(0,0,11,24),LRect(0,24,11,11),LRect(0,53,11,2)}
#define TOPLEVEL_TOPBAR_HEIGHT 29
#define TOPLEVEL_BORDER_RADIUS 11

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
