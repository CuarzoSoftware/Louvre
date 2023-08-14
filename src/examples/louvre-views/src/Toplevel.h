#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <LToplevelRole.h>

class ToplevelView;
class Output;

class Toplevel : public LToplevelRole
{
public:
    Toplevel(Params *params);
    ~Toplevel();

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
    void appIdChanged() override;
    void titleChanged() override;


    void unsetFullscreen();

    ToplevelView *decoratedView = nullptr;
    bool changingState = false;

    LRect prevRect, dstRect;
    Output *fullscreenOutput = nullptr;
};

#endif // TOPLEVEL_H
