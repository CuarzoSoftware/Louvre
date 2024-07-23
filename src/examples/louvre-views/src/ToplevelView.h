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

class ToplevelView final : public LLayerView
{
public:
    ToplevelView(Toplevel *toplevel);
    ~ToplevelView();

    Toplevel *toplevel { nullptr };
    bool lastActiveState { false };
    bool lastFullscreenState { false };

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

    std::string prevClippedTitle;
    LSize unclippedTitleBufferSize;
    LTextureView title;
    UInt32 lastTopbarClickMs { 0 };
    Float32 fullscreenTopbarVisibility { 0.f };
    LAnimation fullscreenTopbarAnim;

    void updateTitle();
    void updateGeometry();

    bool nativeMapped() const noexcept override;
    const LPoint &nativePos() const noexcept override;

    void keyEvent(const LKeyboardKeyEvent &event) override;
};

#endif // TOPLEVELVIEW_H
