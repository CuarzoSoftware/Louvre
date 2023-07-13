#ifndef SURFACE_H
#define SURFACE_H

#include <LSurface.h>
#include <LSurfaceView.h>
#include <Compositor.h>

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
};

#endif // SURFACE_H
