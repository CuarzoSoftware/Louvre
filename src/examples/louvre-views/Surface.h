#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <LSurfaceView.h>
#include <Compositor.h>
#include <LSolidColorPainterMask.h>

using namespace Louvre;
using namespace std;

class Surface : public LSurface
{
public:
    Surface(LSurface::Params *params, GLuint textureUnit = 1);
    ~Surface();
    Compositor *compositor() const;

    void mappingChanged() override;
    void orderChanged() override;
    void roleChanged() override;
    void bufferSizeChanged() override;
    void minimizedChanged() override;

    bool firstMap = true;
    LSurfaceView *view = nullptr;

    LPoint posBeforeMinimized;
    LTexture *minimizedTexture;
    std::list<LTextureView*>minimizedViews;

    LAnimation *minimizeAnim = nullptr;

    LSolidColorPainterMask mask0 = LSolidColorPainterMask(0.0, 0.0, 0.0, 0.0, LRect(0,0,100,100));
    LSolidColorPainterMask mask1 = LSolidColorPainterMask(0.0, 0.0, 0.0, 0.0, LRect(0,0,20,20));
    LSolidColorPainterMask mask2 = LSolidColorPainterMask(0.0, 0.0, 0.0, 0.0, LRect(0,0,20,20));
    LSolidColorPainterMask mask3 = LSolidColorPainterMask(0.0, 0.0, 0.0, 0.0, LRect(0,0,20,20));
};

#endif // SURFACE_H
