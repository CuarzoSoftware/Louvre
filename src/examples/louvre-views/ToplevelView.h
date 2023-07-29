#ifndef TOPLEVELVIEW_H
#define TOPLEVELVIEW_H

#include <LLayerView.h>

class Toplevel;

class ToplevelView : public LLayerView
{
public:
    ToplevelView(Toplevel *toplevel);

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

    void updateGeometry();
    void pointerEnterEvent(const LPoint &localPos) override;
    void pointerButtonEvent(LPointer::Button button, LPointer::ButtonState state) override;

    const LPoint &nativePos() const override;
};

#endif // TOPLEVELVIEW_H
