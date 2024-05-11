#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LSolidColorView.h>
#include <LRenderBuffer.h>
#include <LSceneView.h>
#include <LAnimation.h>
#include <LGammaTable.h>

using namespace Louvre;

class Compositor;
class Dock;
class Toplevel;
class Topbar;
class Workspace;

class Output final : public LOutput
{
public:
    Output(const void *params);

    void initializeGL() override;
    void resizeGL() override;
    void moveGL() override;
    void paintGL() override;
    void uninitializeGL() override;
    void setGammaRequest(LClient *client, const LGammaTable *gamma) override;

    void loadWallpaper();
    void setWorkspace(Workspace *ws, UInt32 animMs, Float64 curve = 2.0, Float64 start = 0.0);
    void updateWorkspacesPos();
    void updateFractionalOversampling();
    void showAllWorkspaces();
    void hideAllWorkspacesExceptCurrent();

    // Current workspace
    Workspace *currentWorkspace = nullptr;

    // Workspace switch/restore animation
    LAnimation workspaceAnim;
    Float64 easingCurve { 2.0 };
    Float64 animStart { 0.0 };
    Toplevel *animatedFullscreenToplevel = nullptr;

    // X workspaces offset
    Float64 workspaceOffset { 0.0 };

    // True if 3 finger swipe
    bool swipingWorkspace = false;

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
