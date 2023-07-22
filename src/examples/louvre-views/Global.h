#ifndef SHARED_H
#define SHARED_H

class Compositor;
class Output;

#include <LNamespaces.h>

using namespace Louvre;

#define DOCK_ITEM_HEIGHT 42
#define DOCK_SPACING 8
#define DOCK_PADDING 8
#define DOCK_MARGIN 8

#define TOPBAR_HEIGHT 24

class G
{
public:

    struct BorderRadiusTextures
    {
        LTexture *TL;
        LTexture *TR;
        LTexture *BR;
        LTexture *BL;
    };

    struct Cursors
    {
        LXCursor *handCursor = nullptr;
    };

    // Quick handles
    static Compositor *compositor();
    static LScene *scene();

    // Output
    static std::list<Output*>&outputs();
    static void arrangeOutputs();

    // Decoration
    static void createBorderRadiusTextures();
    static BorderRadiusTextures *borderRadius();

    // Cursors
    static void loadCursors();
    static Cursors &cursors();
};

#endif // SHARED_H
