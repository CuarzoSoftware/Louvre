#ifndef SHARED_H
#define SHARED_H

class Compositor;
class Output;

#include <LNamespaces.h>

using namespace Louvre;

#define DOCK_ITEM_HEIGHT 48
#define DOCK_SPACING 8
#define DOCK_PADDING 8

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

    // Quick handles
    static Compositor *compositor();

    // Output
    static std::list<Output*>outputs();
    static void arrangeOutputs();

    // Decoration
    static void createBorderRadiusTextures();
    static BorderRadiusTextures *borderRadius();
};

#endif // SHARED_H
