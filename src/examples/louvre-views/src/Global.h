#ifndef SHARED_H
#define SHARED_H

#include <map>
#include <LNamespaces.h>
#include <LRegion.h>
#include <LFramebuffer.h>

using namespace Louvre;

#define TOPBAR_HEIGHT 26

#define DOCK_SHADOW_SIZE 40
#define DOCK_SPACING 4
#define DOCK_PADDING 4
#define DOCK_MARGIN 8
#define DOCK_ITEM_HEIGHT (DOCK_HEIGHT - (2 * DOCK_PADDING))

#define DOCK_HEIGHT 53
#define DOCK_BORDER_RADIUS 16
#define DOCK_APP_DOT_SIZE 5
#define TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_X -48
#define TOPLEVEL_ACTIVE_TOP_LEFT_OFFSET_Y -60
#define TOPLEVEL_ACTIVE_TOP_CLAMP_OFFSET_Y 1
#define TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_X -48
#define TOPLEVEL_ACTIVE_BOTTOM_LEFT_OFFSET_Y -21
#define TOPLEVEL_ACTIVE_MIN_WIDTH 68
#define TOPLEVEL_ACTIVE_MIN_WIDTH_TOP 44
#define TOPLEVEL_ACTIVE_MIN_WIDTH_BOTTOM 68
#define TOPLEVEL_ACTIVE_MIN_HEIGHT 35
#define TOPLEVEL_ACTIVE_TOP_LEFT_TRANS_REGION {LRect(0,0,48,74),LRect(48,0,22,32),LRect(48,31,11,11),LRect(48,60,22,14)}
#define TOPLEVEL_ACTIVE_TOP_RIGHT_TRANS_REGION {LRect(22,0,48,74),LRect(0,0,22,32),LRect(11,31,11,11),LRect(0,60,22,14)}
#define TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_X -41
#define TOPLEVEL_INACTIVE_TOP_LEFT_OFFSET_Y -55
#define TOPLEVEL_INACTIVE_TOP_CLAMP_OFFSET_Y 1
#define TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_X -41
#define TOPLEVEL_INACTIVE_BOTTOM_LEFT_OFFSET_Y -14
#define TOPLEVEL_INACTIVE_MIN_WIDTH 60
#define TOPLEVEL_INACTIVE_MIN_WIDTH_TOP 30
#define TOPLEVEL_INACTIVE_MIN_WIDTH_BOTTOM 60
#define TOPLEVEL_INACTIVE_MIN_HEIGHT 22
#define TOPLEVEL_INACTIVE_TOP_LEFT_TRANS_REGION {LRect(0,0,41,63),LRect(41,0,15,27),LRect(41,26,11,11),LRect(41,55,15,8)}
#define TOPLEVEL_INACTIVE_TOP_RIGHT_TRANS_REGION {LRect(15,0,41,63),LRect(0,0,15,27),LRect(4,26,11,11),LRect(0,55,15,8)}
#define TOPLEVEL_TOPBAR_HEIGHT 29
#define TOPLEVEL_BORDER_RADIUS 11
#define TOPLEVEL_BUTTON_SIZE 12
#define TOPLEVEL_BUTTON_SPACING 8
#define TOPLEVEL_RESIZE_INPUT_MARGIN 4
#define CONTAINER_BORDER_RADIUS 6
#define CONTAINER_TOP_LEFT_SIZE 21
#define CONTAINER_OFFSET -15
#define CONTAINER_ARROW_WIDTH 29
#define CONTAINER_ARROW_HEIGHT 17

class Compositor;
class Output;
class Pointer;
class TextRenderer;
class App;
class Tooltip;
class Surface;

enum ID
{
    WallpaperType = 1,
    DockSeparatorType = 2,
    DockItemType = 3,
    DockAppType = 4
};

class G
{
public:

    enum TextureConfIndex : UInt32
    {
        /* Toplevel Buttons */
        ButtonDisabled                      = 0,
        CloseButtonEnabled                  = 1,
        CloseButtonEnabledHover             = 2,
        CloseButtonEnabledPressed           = 3,
        MinimizeButtonEnabled               = 4,
        MinimizeButtonEnabledHover          = 5,
        MinimizeButtonEnabledPressed        = 6,
        MaximizeButtonEnabled               = 7,
        MaximizeButtonEnabledHover          = 8,
        MaximizeButtonEnabledPressed        = 9,
        FullscreenButtonEnabledHover        = 10,
        FullscreenButtonEnabledPressed      = 11,
        UnfullscreenButtonEnabledHover      = 12,
        UnfullscreenButtonEnabledPressed    = 13,

        /* Toplevel Decoration */
        DecorationActiveTL                  = 14,
        DecorationActiveTR                  = 15,
        DecorationActiveT                   = 16,
        DecorationActiveL                   = 17,
        DecorationActiveR                   = 18,
        DecorationActiveBL                  = 19,
        DecorationActiveBR                  = 20,
        DecorationActiveB                   = 21,
        DecorationInactiveTL                = 22,
        DecorationInactiveTR                = 23,
        DecorationInactiveT                 = 24,
        DecorationInactiveL                 = 25,
        DecorationInactiveR                 = 26,
        DecorationInactiveBL                = 27,
        DecorationInactiveBR                = 28,
        DecorationInactiveB                 = 29,
        DecorationMaskBL                    = 30,
        DecorationMaskBR                    = 31,

        /* Tooltip */
        TooltipT                            = 32,
        TooltipR                            = 33,
        TooltipB                            = 34,
        TooltipL                            = 35,
        TooltipTL                           = 36,
        TooltipTR                           = 37,
        TooltipBR                           = 38,
        TooltipBL                           = 39,
        TooltipArrow                        = 40,

        /* Dock */
        DockL                               = 41,
        DockC                               = 42,
        DockR                               = 43,
        DockDot                             = 44,

        /* Louvre Logo */
        Logo                                = 45
    };

    struct Cursors
    {
        LXCursor *arrow = nullptr;
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

    struct ToplevelRegions
    {
        LRegion activeTransRegionTL;
        LRegion activeTransRegionTR;
        LRegion inactiveTransRegionTL;
        LRegion inactiveTransRegionTR;
    };

    struct TextureViewConf
    {
        LTexture *texture = nullptr;
        LRectF customSrcRect;
        Float32 bufferScale = 1.f;
        LSize customDstSize;
        LFramebuffer::Transform transform = LFramebuffer::Normal;
        LRGBF customColor;
        bool enableCustomColor = false;
        bool enableCustomSrcRect = true;
        bool enableCustomDstSize = true;
    };

    struct Textures
    {
        // Louvre label
        LTexture *defaultTopbarAppName = nullptr;

        // Terminal icon
        LTexture *defaultAppIcon = nullptr;

        // UI texture
        LTexture *atlas;

        // UI textures confs
        TextureViewConf UIConf[46];
    };

    struct Fonts
    {
        TextRenderer *regular = nullptr;
        TextRenderer *semibold = nullptr;
    };

    // Quick handles
    static Compositor *compositor();
    static LScene *scene();
    static Pointer *pointer();
    static const std::vector<Output*>&outputs();
    static std::list<Surface*>&surfaces();

    // Dock
    static void enableDocks(bool enabled);
    static void loadApps();
    static std::vector<App *> &apps();

    // Tooltip
    static void createTooltip();
    static Tooltip *tooltip();

    // Cursors
    static void loadCursors();
    static Cursors &cursors();

    // Textures
    static Textures *textures();
    static void loadTextures();
    static void setTexViewConf(LTextureView *view, UInt32 index);

    // Toplevel regions
    static ToplevelRegions *toplevelRegions();
    static void loadToplevelRegions();

    // Fonts
    static void loadFonts();
    static Fonts *font();
    static const char *transformName(LFramebuffer::Transform transform);

    // Utils
    static LTexture *loadAssetsTexture(const std::filesystem::path &file, bool exitOnFail = true);
    static void enableParentScalingChildren(LView *parent, bool enabled);
    static void enableClippingChildren(LView *parent, bool enabled);
    static Output *mostIntersectedOuput(LView *view);
    static void reparentWithSubsurfaces(Surface *surf, LView *newParent, bool onlySubsurfaces = true);
    static void arrangeOutputs();
    static class Toplevel *searchFullscreenParent(Surface *parent);
    static void repositionNonVisibleToplevelChildren(Output *target, Surface *toplevel);
};

#endif // SHARED_H
