#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <LToplevelRole.h>

class ToplevelView;

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

    void decorationModeChanged() override;
    void geometryChanged() override;
    void activatedChanged() override;
    void preferredDecorationModeChanged() override;

    ToplevelView *decoratedView = nullptr;

    // Animations
    LAnimation *anim = nullptr;
    LRect prevRect, dstRect;
};

#endif // TOPLEVEL_H
