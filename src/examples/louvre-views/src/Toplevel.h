#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <LTextureView.h>
#include <LToplevelRole.h>
#include <LSolidColorView.h>

class ToplevelView;
class Output;
class Workspace;

using namespace Louvre;

class Toplevel : public LToplevelRole
{
public:
    Toplevel(Params *params);
    ~Toplevel();

    // Quick parse handles
    inline class Surface *surf() const {return (Surface*)surface();};

    const LPoint &rolePos() const override;
    void configureRequest() override;

    void startResizeRequest(ResizeEdge edge) override;
    void startMoveRequest() override;

    void setMaximizedRequest() override;
    void unsetMaximizedRequest() override;
    void maximizedChanged() override;

    void setFullscreenRequest(LOutput *output) override;
    void unsetFullscreenRequest() override;
    void fullscreenChanged() override;

    void setMinimizedRequest() override;

    void decorationModeChanged() override;
    void preferredDecorationModeChanged() override;

    void geometryChanged() override;

    void activatedChanged() override;
    void titleChanged() override;

    void unsetFullscreen();

    bool destructorCalled = false;
    bool quickUnfullscreen = false;

    ToplevelView *decoratedView = nullptr;

    LSolidColorView blackFullscreenBackground;

    // Rendered view for fullscreen animation
    LTextureView capture;

    // Rects for fullscreen animation
    LRect prevRect, dstRect, prevBoundingRect;
    Output *fullscreenOutput = nullptr;
    Workspace *fullscreenWorkspace = nullptr;
    UInt32 prevStates = 0;
    UInt32 outputUnplugConfigureCount = 0;

    LTextureView animView;
    LSceneView *animScene = nullptr;
};

#endif // TOPLEVEL_H
