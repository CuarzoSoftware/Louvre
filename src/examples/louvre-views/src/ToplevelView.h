#ifndef TOPLEVELVIEW_H
#define TOPLEVELVIEW_H

#include <LLayerView.h>
#include <LSceneView.h>
#include <LSurfaceView.h>
#include <LAnimation.h>
#include "UITextureView.h"
#include "InputRect.h"
#include "ToplevelButton.h"

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
    LSurfaceView surfB;

    LSceneView sceneBL;
    LSceneView sceneBR;

    LSurfaceView surfBL;
    LSurfaceView surfBR;

    UITextureView decoTL, decoT, decoTR, decoL,
        decoR, decoBL, decoB, decoBR,
        maskBL, maskBR;

    InputRect
        topbarInput,
        buttonsContainer,
        resizeT,
        resizeB,
        resizeL,
        resizeR,
        resizeTL,
        resizeTR,
        resizeBL,
        resizeBR;

    ToplevelButton closeButton,
        minimizeButton,
        maximizeButton;

    LTextureView title;

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
