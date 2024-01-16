#ifndef TOPLEVELVIEW_H
#define TOPLEVELVIEW_H

#include <LLayerView.h>
#include <LSceneView.h>
#include <LSurfaceView.h>
#include <LAnimation.h>
#include "UITextureView.h"

using namespace Louvre;

class Toplevel;
class InputRect;
class ToplevelButton;

class ToplevelView : public LLayerView
{
public:
    ToplevelView(Toplevel *toplevel);
    ~ToplevelView();

    Toplevel *toplevel = nullptr;
    bool lastActiveState = false;
    bool lastFullscreenState = false;

    LLayerView clipTop;
    LLayerView clipBottom;
    LSurfaceView *surfB = nullptr;

    LSceneView sceneBL;
    LSceneView sceneBR;

    LSurfaceView surfBL;
    LSurfaceView surfBR;

    UITextureView decoTL, decoT, decoTR, decoL,
        decoR, decoBL, decoB, decoBR,
        maskBL, maskBR;

    InputRect *resizeTL = nullptr;
    InputRect *resizeTR = nullptr;
    InputRect *resizeBL = nullptr;
    InputRect *resizeBR = nullptr;
    InputRect *resizeT = nullptr;
    InputRect *resizeB = nullptr;
    InputRect *resizeL = nullptr;
    InputRect *resizeR = nullptr;
    InputRect *topbarInput = nullptr;

    InputRect *buttonsContainer = nullptr;
    ToplevelButton *closeButton = nullptr;
    ToplevelButton *minimizeButton = nullptr;
    ToplevelButton *maximizeButton = nullptr;

    LTextureView *title = nullptr;

    UInt32 lastTopbarClickMs = 0;
    Float32 fullscreenTopbarVisibility = 0.f;
    LAnimation fullscreenTopbarAnim;

    Int32 titleWidth = 0;
    void updateTitle();
    void updateGeometry();

    bool nativeMapped() const override;
    const LPoint &nativePos() const override;

    void keyEvent(UInt32 keyCode, UInt32 keyState) override;
};

#endif // TOPLEVELVIEW_H
