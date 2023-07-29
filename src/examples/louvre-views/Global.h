#ifndef SHARED_H
#define SHARED_H

class Compositor;
class Output;

#include <LNamespaces.h>

using namespace Louvre;

#define DOCK_BUFFER_SCALE 2
#define DOCK_SHADOW_SIZE 40
#define DOCK_HEIGHT 53
#define DOCK_SPACING 8
#define DOCK_PADDING 8
#define DOCK_MARGIN 8
#define DOCK_BORDER_RADIUS 16
#define DOCK_ITEM_HEIGHT (DOCK_HEIGHT - (2 * DOCK_PADDING))


#define TOPBAR_HEIGHT 24

#define TOPLEVEL_TOPBAR_HEIGHT 24
#define TOPLEVEL_TOPBAR_BUTTON_SIZE 12

class G
{
public:

    struct Cursors
    {
        LXCursor *hand2 = nullptr;
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
