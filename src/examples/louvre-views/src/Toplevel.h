#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <LTextureView.h>
#include <LToplevelRole.h>
#include <LSolidColorView.h>
#include <LRegion.h>

class ToplevelView;
class Output;
class Workspace;

using namespace Louvre;

class Toplevel final : public LToplevelRole
{
public:
    Toplevel(const void *params);
    ~Toplevel();

    // Quick parse handles
    inline class Surface *surf() const {return (Surface*)surface();}

    const LPoint &rolePos() const override;
    void configureRequest() override;
    void configurationChanged(LBitset<ConfigurationChanges> changes) override;
    void startResizeRequest(const LEvent &triggeringEvent, ResizeEdge edge) override;
    void startMoveRequest(const LEvent &triggeringEvent) override;
    void setMaximizedRequest() override;
    void unsetMaximizedRequest() override;
    void maximizedChanged();
    void activatedChanged();
    void setFullscreenRequest(LOutput *output) override;
    void unsetFullscreenRequest() override;
    void fullscreenChanged();
    void setMinimizedRequest() override;
    void preferredDecorationModeChanged() override;
    void decorationModeChanged();
    void titleChanged() override;
    void unsetFullscreen();

    bool requestedFullscreenOnFirstMap { false };
    bool destructorCalled { false };
    bool quickUnfullscreen { false };

    ToplevelView *decoratedView { nullptr };

    LSolidColorView blackFullscreenBackground;

    // Rendered view for fullscreen animation
    LTextureView capture;
    LRegion captureTransRegion;

    // Rects for fullscreen animation
    LRect prevRect, dstRect, prevBoundingRect;
    Output *fullscreenOutput { nullptr };
    Workspace *fullscreenWorkspace { nullptr };
    UInt32 prevStates { 0 };
    UInt32 outputUnplugConfigureCount { 0 };

    LTextureView animView;
    LSceneView *animScene { nullptr };
};

#endif // TOPLEVEL_H
