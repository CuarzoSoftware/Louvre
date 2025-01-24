#ifndef OUTPUT_H
#define OUTPUT_H

#include <LOutput.h>
#include <LSolidColorView.h>
#include <LRenderBuffer.h>
#include <LSceneView.h>
#include <LAnimation.h>

#include "Topbar.h"
#include "Dock.h"

using namespace Louvre;

class Compositor;
class Toplevel;
class Workspace;
class LayerRole;

class Output final : public LOutput
{
public:
    Output(const void *params) noexcept;

    void initializeGL() override;
    void resizeGL() override;
    void moveGL() override;
    void paintGL() override;
    void uninitializeGL() override;
    void setGammaRequest(LClient *client, const LGammaTable *gamma) override;

    void setWorkspace(Workspace *ws, UInt32 animMs, Float64 curve = 2.0, Float64 start = 0.0);
    void updateWorkspacesPos();
    void updateFractionalOversampling();
    bool tryFullscreenScanoutIfNoOverlayContent() noexcept;
    void showAllWorkspaces();
    void hideAllWorkspacesExceptCurrent();

    void onWorkspacesAnimationUpdate(LAnimation *anim) noexcept;
    void onWorkspacesAnimationFinish(LAnimation *anim) noexcept;

    // The main workspace and one for each fullscreen toplevel
    std::list<Workspace*> workspaces;
    LAnimation workspacesAnimation { 400,
            [this](LAnimation *anim){ onWorkspacesAnimationUpdate(anim); },
            [this](LAnimation *anim){ onWorkspacesAnimationFinish(anim); }};
    LLayerView workspacesContainer;
    Float64 workspacesPosX  { 0.0 };
    LWeak<Workspace> currentWorkspace;
    Float64 animStartOffset { 0.0 };
    Float64 animEasingCurve { 2.0 };
    LWeak<Toplevel> animatedFullscreenToplevel;
    bool workspaceAnimationInFirstFrame { true };
    bool doingFingerWorkspaceSwipe { false };

    Topbar topbar { this };
    Dock dock { this };
    LWeak<LayerRole> shelf;

    void updateWallpaper() noexcept;
    LTextureView wallpaper;
    /* Used only with the DRM backend */
    std::unique_ptr<LTexture> scaledWallpaper;

    // Zoom
    void zoomedDrawBegin() noexcept;
    void zoomedDrawEnd() noexcept;
    void setZoom(Float32 zoom) noexcept;
    Float32 zoom { 1.f };
    LSceneView zoomScene { LSize(0, 0), 1.f };
    LTextureView zoomView;
};

#endif // OUTPUT_H
