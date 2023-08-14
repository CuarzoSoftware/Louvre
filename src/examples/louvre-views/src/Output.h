#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LSolidColorView.h>
#include <LRenderBuffer.h>
#include <LSceneView.h>

using namespace Louvre;

class Compositor;
class Dock;
class Toplevel;
class Topbar;
class Workspace;

class Output : public LOutput
{
public:
    Output();

    void initializeGL() override;
    void resizeGL() override;
    void moveGL() override;
    void paintGL() override;
    void uninitializeGL() override;

    void loadWallpaper();

    // Main workspace and one for each fullscreen toplevel
    std::list<Workspace*>workspaces;

    // Topbar for this output
    Topbar *topbar = nullptr;

    // Wallpaper for this output
    LTextureView *wallpaperView = nullptr;

    // Dock for this output
    Dock *dock = nullptr;

    // Current fullscreen toplevel on this output
    Toplevel *fullscreenToplevel = nullptr;

    // Black view used as background for fullscreen Toplevels
    LSolidColorView *fullscreenView = nullptr;
};

#endif // OUTPUT_H
