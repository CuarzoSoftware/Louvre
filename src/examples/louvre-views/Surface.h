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
    bool firstMap = true;
    LSurfaceView view = LSurfaceView(this, true, false, (LView*)&compositor()->scene.mainView());
};

#endif // SURFACE_H
