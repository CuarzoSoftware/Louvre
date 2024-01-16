#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LSolidColorView.h>
#include <LRenderBuffer.h>
#include <LSceneView.h>
#include <LAnimation.h>

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

    void setWorkspace(Workspace *ws, UInt32 animMs, Float32 curve = 2.f, Float32 start = 0.f);
    void updateWorkspacesPos();
    void updateFractionalOversampling();

    // Current workspace
    Workspace *currentWorkspace = nullptr;

    // Workspace switch/restore animation
    LAnimation workspaceAnim;
    Float32 easingCurve = 2.f;
    Float32 animStart = 0.f;
    Toplevel *animatedFullscreenToplevel = nullptr;

    // X workspaces offset
    Float32 workspaceOffset = 0.f;

    // True if 3 finger swipe
    bool swippingWorkspace = false;

    // Main workspace and one for each fullscreen toplevel
    std::list<Workspace*>workspaces;

    // Container to move workspaces
    LLayerView *workspacesContainer = nullptr;

    // Topbar for this output
    Topbar *topbar = nullptr;

    // Wallpaper for this output
    LTextureView *wallpaperView = nullptr;

    // Dock for this output
    Dock *dock = nullptr;
};

#endif // OUTPUT_H
