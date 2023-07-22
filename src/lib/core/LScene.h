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
    void handleMoveGL(LOutput *output);
    void handleResizeGL(LOutput *output);
    void handleUninitializeGL(LOutput *output);

    // Pointer events
    LView *handlePointerMoveEvent(Float32 dx, Float32 dy, LPoint *localPos = nullptr);
    LView *handlePointerPosChangeEvent(Float32 x, Float32 y, LPoint *localPos = nullptr);
    void handlePointerButtonEvent(LPointer::Button button, LPointer::ButtonState state);
    void handlePointerAxisEvent(Float64 axisX, Float64 axisY, Int32 discreteX, Int32 discreteY, UInt32 source);
    bool handleWaylandPointerEventsEnabled() const;
    void enableHandleWaylandPointerEvents(bool enabled);


    LSceneView *mainView() const;
    LView *viewAt(const LPoint &pos);

LPRIVATE_IMP(LScene)
};

#endif // LSCENE_H
