#ifndef TOPLEVELVIEW_H
#define TOPLEVELVIEW_H

#include <LLayerView.h>

class Toplevel;
class InputRect;
class ToplevelButton;

class ToplevelView : public LLayerView
{
public:
    ToplevelView(Toplevel *toplevel);
    ~ToplevelView();

    bool lastActiveState = false;
    bool lastFullscreenState = false;
    Toplevel *toplevel = nullptr;

    LLayerView *clipTop = nullptr;
    LLayerView *clipBottom = nullptr;
    LSurfaceView *surfB = nullptr;

    LTextureView *decoTL = nullptr;
    LTextureView *decoT = nullptr;
    LTextureView *decoTR = nullptr;
    LTextureView *decoL = nullptr;
    LTextureView *decoR = nullptr;
    LTextureView *decoBL = nullptr;
    LTextureView *decoB = nullptr;
    LTextureView *decoBR = nullptr;

    LTextureView *maskBL = nullptr;
    LTextureView *maskBR = nullptr;

    LSceneView *sceneBL = nullptr;
    LSceneView *sceneBR = nullptr;

    LSurfaceView *surfBL = nullptr;
    LSurfaceView *surfBR = nullptr;

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

    Int32 titleWidth = 0;
    void updateTitle();
    void updateGeometry();

    bool nativeMapped() const override;
    const LPoint &nativePos() const override;

    void keyEvent(UInt32 keyCode, UInt32 keyState) override;
};

#endif // TOPLEVELVIEW_H
