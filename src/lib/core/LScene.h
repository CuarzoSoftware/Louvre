#ifndef LSCENE_H
#define LSCENE_H

#include <LObject.h>
#include <LPointer.h>

class Louvre::LScene : public LObject
{
public:
    LScene();
    ~LScene();

    // Output
    void handleInitializeGL(LOutput *output);
    void handlePaintGL(LOutput *output);
    void handleResizeGL(LOutput *output);
    void handleUninitializeGL(LOutput *output);

    LLayerView &mainView() const;
    const LRGBF &clearColor() const;
    void setClearColor(Float32 r, Float32 g, Float32 b);
    void setClearColor(const LRGBF &color);
    LView *viewAtC(const LPoint &pos);

LPRIVATE_IMP(LScene)
};

#endif // LSCENE_H
